#include "GameInstance.h"

#include <chrono>
#include <thread>
#include <algorithm>
#include "json.h"
#include "MapHelper.h"

constexpr auto EVENT_GRADUATE = 45;

using namespace std;

void TableInstance::init(const unordered_map<int, ClientState>& clients) {
	waitingIntervalInMilliseconds = 100;
	switch (settings.getSpeed())
	{
	case 1:
		moveIntervalInMilliseconds = 1000; break;
	case 2:
		moveIntervalInMilliseconds = 700; break;
	case 3:
		moveIntervalInMilliseconds = 500; break;
	case 4:
		moveIntervalInMilliseconds = 300; break;
	default:
		moveIntervalInMilliseconds = 700; break;
	}
	maximumWaitingTimeInMilliseconds = 1000 * settings.getWaitTime();

	players.clear();
	for (auto it = clients.cbegin(); it != clients.cend(); ++it) {
		players.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(it->first),
			std::forward_as_tuple(it->first, it->second == ClientState::Robot,
				[this]() { sendState(); },
				[this](HintType hintType, std::initializer_list<int64_t> param) { sendHint(hintType, param); }));
	}
	for (auto it = players.begin(); it != players.end(); ++it) playerInit(it->second);

	cardListInit();
}

void TableInstance::start() {
	cycle();
}

void TableInstance::checkBankruptcy() {
	for (auto it = players.begin(); it != players.end(); ++it) {
		if (it->second.money < 0) {
			sendHint(HintType::Bankruptcy, { it->first });
			playerBankrupt(it->second);
		}
	}
}

void TableInstance::cycle() {
	while (!checkGameOver()) {
		for (auto it = players.begin(); it != players.end(); ++it) {
			setActivePlayer(it->first);
			sendHint(HintType::StartRound, { active });
			Player& p = players.at(active);
			if (p.cantMove) {
				p.cant_move_modifier(false);
				continue;
			}
			action();
		}
		checkBankruptcy();
	}
	terminate();
}

void TableInstance::setActivePlayer(int pid) {
	active = pid;
	sendState();
}

void TableInstance::action() {
	Action step(ActionType::DefaultStart);

	do {
		step = request(step);
		crash();
	} while (step);
}

Action TableInstance::request(Action current) {
	Player& p = players.at(active);
	int pid = active;
	int eid = getEventId(current, p.position);
	while (true) {
		p.state == PlayerState::Robot ? oneTimeRequestRobot(pid, eid) : oneTimeRequest(pid, eid);
		if (checkEvents(current)) return handleEvents(current);
		sendHint(HintType::ResponseError, { pid });
	}
}

void TableInstance::terminate() {
	json data(unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::TableEnd } },
		{ "D", capture() }
	});

	for (auto it = players.cbegin(); it != players.cend(); ++it) {
		send(it->first, data);
	}
}

bool TableInstance::checkGameOver() const {
	for (auto it = players.cbegin(); it != players.cend(); ++it) {
		if (playerScore(it->second) >= settings.getScoreTarget()) {
			return true;
		}
	}
	return false;
}

void TableInstance::crash() {
	Player& p = players.at(active);

	if (p.position->pointType == PointType::Corner) return;

	for (auto it = players.begin(); it != players.end(); ++it) {
		if (it->first != p.pid && it->second.position == p.position) {
			sendHint(HintType::Crash, { p.pid, it->first });
			playerMove(it->second, true, 2);
		}
	}
}

int TableInstance::roll(int num) const {
	int result = 0;
	random_device rd; mt19937 mt_rand(rd());
	for (int i = 0; i < num; ++i) {
		result = result + mt_rand() % 6 + 1;
	}
	sendHint(HintType::Roll, { active, num, result });
	return result;
}

int TableInstance::getEventId(Action action, const MapPoint* position) const {
	switch (action.actionType)
	{
	case ActionType::DefaultStart:
		return position->eid_start;
	case ActionType::DefaultEnd:
		return position->eid_end;
	case ActionType::Extra:
		return action.specialEventId;
	case ActionType::None:
		return 0;
	}
	return 0;
}

json TableInstance::capture() const {
	unordered_map<string, json> playerMap;
	for (auto it = players.cbegin(); it != players.cend(); ++it) {
		playerMap[to_string(it->first)] = it->second.capture();
		playerMap[to_string(it->first)]["EC"] = { collect(true, it->first) };
		playerMap[to_string(it->first)]["CC"] = { collect(false, it->first) };
	}
	return { unordered_map<string, json> {
		{ "PM", { move(playerMap) } },
		{ "PIA", { active } } } };
}

TableInstance::TableInstance(const function<void(int, const json&)> send) :
	active(0),
	maximumWaitingTimeInMillisecondsForAutoPlayer(2000),
	maximumWaitingTimeInMilliseconds(30000),
	waitingIntervalInMilliseconds(1000),
	moveIntervalInMilliseconds(1000),
	send(send) {
} 

TableInstance::~TableInstance() {

}

bool TableInstance::checkEvents(Action action) const {
	const Player& p = players.at(active);
	int eid = getEventId(action, p.position);
	switch (eid)
	{
	// Todo: Card Condition Unchecked
	case 0:
		return
			(event[0] == 1) ||
			(event[0] == 2 && chCardMap.at(event[1]) == p.pid) ||
			(event[0] == 3 && exCardMap.at(event[1]) == p.pid) ||
			(event[0] == 4 && p.retireTimes > 0);
	case 5:
		return 
			(event[0] == 1 && p.money >= 4000) ||
			(event[0] == 2 && p.money >= 8000) ||
			(event[0] == 3 && p.money >= 12000) ||
			(event[0] == 4);
	case 6:
		return
			(event[0] == 1 && (
				p.degree == DegreeType::Science ||
				p[CareerType::Moon] > 0 ||
				p.money >= 5000)) ||
			(event[0] == 2);
	case 7:
		return
			(event[0] == 1 && p.money >= 2000) ||
			(event[0] == 2 && p.money >= 8000) ||
			(event[0] == 3 && p.money >= 16000) ||
			(event[0] == 4);
	case 8:
		return
			(event[0] == 1 && (
				p.degree == DegreeType::Engineering ||
				p[CareerType::Miner] > 0 ||
				p.money >= 4000)) ||
			(event[0] == 2);
	case 9:
		return
			(event[0] == 1 && p.money >= 3000) ||
			(event[0] == 2);
	case 11:
		return
			(event[0] == 1 && (
				!(p.degree == DegreeType::None) ||
				p[CareerType::Company] > 0 ||
				p.money >= 500)) ||
			(event[0] == 2);
	case 12:
		return
			(event[0] == 1 && p.money >= 3000) ||
			(event[0] == 2 && p.money >= 6000) ||
			(event[0] == 3);
	case 13:
		return
			(event[0] == 1 && (
				p[CareerType::Sea] > 0 ||
				p.money >= 100)) ||
			(event[0] == 2);
	case 15:
		return
			(event[0] == 1 && (
				p[CareerType::Farmer] > 0 ||
				p.money >= 1000)) ||
			(event[0] == 2);
	case 16:
		return
			(event[0] == 1 && p.money >= p.salary) ||
			(event[0] == 2);
	case 17:
		return
			(event[0] == 1 && (
				!(p.degree == DegreeType::None) ||
				p.money >= 500)) ||
			(event[0] == 2);
	case 18:
		return
			(event[0] == 1 && (
				p[CareerType::Movie] > 0 ||
				p.money >= 1000)) ||
			(event[0] == 2);
	case 20:
		return
			(event[0] == 1 && (
				p.degree == DegreeType::Law ||
				p[CareerType::Politic] > 0 ||
				p.money >= 3000)) ||
			(event[0] == 2);
	case 26:
		return
			(event[0] == 1 && p.money >= 5000) ||
			(event[0] == 2);
	case 42:
		return
			(event[0] == 1 && p.money >= p.salary / 2) ||
			(event[0] == 2);
	case 45:
		return playerWithDegree((DegreeType)event[0]) == 0u;
	case 1:
	case 2:
	case 3:
	case 4:
	case 10:
	case 14:
	case 19:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
	case 43:
	case 44:
	case 100:
		return true;
	default:
		return false;
	}
}

Action TableInstance::handleEvents(Action action) {
	Player& p = players.at(active);
	int eid = getEventId(action, p.position);
	switch (eid)
	{
	case 0: {
		if (event[0] == 1) {
			if (p.position->pointType == PointType::Career || p.careerSelect)
				playerMove(p, false, roll(1));
			else
				playerMove(p, false, roll(2));
			return { ActionType::DefaultEnd };
		}
		else if (event[0] == 2) {
			int step = 0;
			const ChCard& card = CardDesk::chCards[event[1]];
			ChCardType cardType = card.chCardType == ChCardType::NormalAny ? (ChCardType)event[2] : card.chCardType;

			if (cardType == ChCardType::NormalStudy && !(p.degree == DegreeType::None)) {
				drawCard(false, p.pid);
				chCardMap[card.cid] = 0u;
				discardChList.push_back(card.cid);
				sendState();
				sendHint(HintType::UseChCard, { p.pid, card.cid });
				return { 0 };
			}

			switch (cardType)
			{
			case ChCardType::NormalCompany:
				if (p.degree == DegreeType::None && p[CareerType::Company] == 0) {
					p.money_modifier(-500);
				}
				step = 14;
				break;
			case ChCardType::NormalMoon:
				if (p.degree != DegreeType::Science && p[CareerType::Moon] == 0) {
					p.money_modifier(-5000);
				}
				step = 6;
				break;
			case ChCardType::NormalPolitic:
				if (p.degree != DegreeType::Law && p[CareerType::Politic] == 0) {
					p.money_modifier(-3000);
				}
				step = 30;
				break;
			case ChCardType::NormalMovie:
				if (p[CareerType::Movie] == 0) {
					p.money_modifier(-1000);
				}
				step = 27;
				break;
			case ChCardType::NormalMiner:
				if (p.degree != DegreeType::Engineering && p[CareerType::Miner] == 0) {
					p.money_modifier(-4000);
				}
				step = 9;
				break;
			case ChCardType::NormalFarmer:
				if (p[CareerType::Farmer] == 0) {
					p.money_modifier(-1000);
				}
				step = 22;
				break;
			case ChCardType::NormalSea:
				if (p[CareerType::Sea] == 0) {
					p.money_modifier(-100);
				}
				step = 17;
				break;
			case ChCardType::NormalStudy:
				p.money_modifier(-500);
				step = 25;
				break;
			case ChCardType::Holiday:
				step = 3;
				break;
			case ChCardType::SpecialMiner:
				step = 9;
				break;
			case ChCardType::SpecialMovie:
				step = 27;
				break;
			case ChCardType::SpecialMoon:
				step = 6;
				break;
			case ChCardType::SpecialPolitic:
				step = 30;
				break;
			case ChCardType::SpecialSea2X:
				if (p[CareerType::Sea] == 0) {
					p.money_modifier(-100);
				}
				p.double_happiness_modifier(true);
				step = 17;
				break;
			case ChCardType::NormalAny:
				return { ActionType::None };
			default:
				return { ActionType::None };
			}
			chCardMap[card.cid] = 0u;
			discardChList.push_back(card.cid);
			sendState();
			sendHint(HintType::UseChCard, { p.pid, card.cid });
			playerMove(p, true, step);
			p.career_select_modifier(!(card.chCardType == ChCardType::Holiday));
			return { ActionType::None };
		}
		else if (event[0] == 4) {
			p.retire_time_modifier(-1);
			playerMove(p, true, 3);
			return { ActionType::DefaultEnd };
		}
		else if (event[0] == 3) {
			const ExCard& card = CardDesk::exCards[event[1]];
			exCardMap[card.cid] = 0u;
			discardExList.push_back(card.cid);
			sendState();
			sendHint(HintType::UseExCard, { p.pid, card.cid });
			playerMove(p, false, card.step);
			return { ActionType::DefaultEnd };
		}
	}	
	case 42: {
		// 医生直接出院
		if (p.degree == DegreeType::Medicine) {
			return { 0 };
		}
		else if (event[0] == 1) {
			p.money_modifier(-p.salary / 2);
			int doctor = playerWithDegree(DegreeType::Medicine);
			if (doctor) players.at(doctor).money_modifier(p.salary / 2);
			return { 0 };
		}
		else if (event[0] == 2) {
			int rollNum = roll(2);
			playerMove(p, false, rollNum <= 5 ? rollNum : 0);
			return { ActionType::DefaultEnd };
		}
	}
	case 43: {
		if (event[0] == 1) {
			int rollNum = roll(2);
			if (rollNum <= 7)
				p.happiness_modifier(2);
			else
				playerMove(p, false, rollNum);
			return { ActionType::DefaultEnd };
		}
		else if (event[0] == 2) {
			return { 0 };
		}
	}
	case 44: {
		if (event[0] == 1) {
			p.money_modifier(-p.money / 2);
			return { 0 };
		}
		else if (event[0] == 2) {
			int rollNum = roll(2);
			playerMove(p, false, rollNum == 5 || rollNum == 7 || rollNum % 2 == 0 ? rollNum : 0);
			return { ActionType::DefaultEnd };
		}
	}
	case 100: {
		p.money_modifier(p.position->params[0]);
		p.salary_modifier(p.position->params[1]);
		p.happiness_modifier(p.position->params[2] * (p.doubleHappiness ? 2 : 1));
		p.reputation_modifier(p.position->params[3]);
		for (int i = 0; i < p.position->params[4]; ++i) drawCard(true, p.pid);
		for (int i = 0; i < p.position->params[5]; ++i) drawCard(false, p.pid);
		return { ActionType::None };
	}
	case 1:
		p.money_modifier(p.salary);
		return { ActionType::None };
	case 2:
		return { ActionType::None };
	case 3:
		return { ActionType::None };
	case 4:
		return { ActionType::None };
	case 5:
		if (event[0] == 1) {
			p.money_modifier(-4000);
			p.reputation_modifier(4);
		}
		else if (event[0] == 2) {
			p.money_modifier(-8000);
			p.reputation_modifier(8);
		}
		else if (event[0] == 3) {
			p.money_modifier(-12000);
			p.reputation_modifier(12);
		}
		return { ActionType::None };
	case 6:
		if (event[0] == 1) {
			if (!(p.degree == DegreeType::Science || p[CareerType::Moon] > 0)) {
				p.money_modifier(-5000);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 7:
		if (event[0] == 1) {
			p.money_modifier(-2000);
			p.happiness_modifier(4);
		}
		else if (event[0] == 2) {
			p.money_modifier(-8000);
			p.happiness_modifier(8);
		}
		else if (event[0] == 3) {
			p.money_modifier(-16000);
			p.happiness_modifier(12);
		}
		return { ActionType::None };
	case 8:
		if (event[0] == 1) {
			if (!(p.degree == DegreeType::Engineering || p[CareerType::Miner] > 0)) {
				p.money_modifier(-4000);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 9:
		if (event[0] == 1) {
			p.money_modifier(1000 * roll(1) - 3000);
		}
		return { ActionType::None };
	case 10:
		p.money_modifier(-p.money / 4);
		return { ActionType::None };
	case 11:
		if (event[0] == 1) {
			if (!(p.degree != DegreeType::None || p[CareerType::Company] > 0)) {
				p.money_modifier(-500);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 12:
		if (event[0] == 1) {
			p.money_modifier(-3000);
			p.happiness_modifier(roll(1));
		}
		else if (event[0] == 2) {
			p.money_modifier(-6000);
			p.happiness_modifier(roll(2));
		}
		return { ActionType::None };
	case 13:
		if (event[0] == 1) {
			if (!(p[CareerType::Sea] > 0)) {
				p.money_modifier(-100);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 14:
		if (p.salary <= 3000)
			p.money_modifier(-p.salary / 10);
		else if (p.salary < 10000)
			p.money_modifier(-p.salary / 2);
		else
			p.money_modifier(-p.salary * 9 / 10);
		return { ActionType::None };
	case 15:
		if (event[0] == 1) {
			if (!(p[CareerType::Farmer] > 0)) {
				p.money_modifier(-1000);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 16:
		if (event[0] == 1) {
			p.money_modifier(-p.salary);
			p.happiness_modifier(p.salary / 1000);
		}
		else if (event[0] == 2) {
			p.happiness_modifier(-1);
		}
		return { ActionType::None };
	case 17:
		if (event[0] == 1) {
			if (p.degree == DegreeType::None) {
				p.money_modifier(-500);
				p.career_select_modifier(true);
			}
			else {
				return { 0 };
			}
		}
		return { ActionType::None };
	case 18:
		if (event[0] == 1) {
			if (!(p[CareerType::Movie] > 0)) {
				p.money_modifier(-1000);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 19:
		p.money_modifier(-p.money * roll(1) / 10);
		return { ActionType::None };
	case 20:
		if (event[0] == 1) {
			if (!(p.degree == DegreeType::Law || p[CareerType::Politic] > 0)) {
				p.money_modifier(-3000);
			}
			p.career_select_modifier(true);
		}
		return { ActionType::None };
	case 21:
		p.money_modifier(-p.salary / 2);
		return { ActionType::None };
	case 22:
		p.reputation_modifier(-p.reputation);
		return { ActionType::None };
	case 23:
		p.money_modifier(-p.money);
		return { ActionType::None };
	case 24:
		p.reputation_modifier(10);
		playerMove(p, true, 2);
		return { ActionType::DefaultEnd };
	case 25:
		playerMove(p, false, roll(1));
		return { ActionType::DefaultEnd };
	case 26:
		if (event[0] == 1) {
			p.money_modifier(-5000);
			return { ActionType::None };
		}
		else {
			playerMove(p, true, 4);
			return { ActionType::DefaultEnd };
		}
	case 27:
		playerMove(p, true, 2);
		return { ActionType::DefaultEnd };
	case 28:
		p.salary_modifier(-p.salary / 2);
		return { ActionType::None };
	case 29:
		p.reputation_modifier(10);
		p.happiness_modifier(-p.happiness);
		return { ActionType::None };
	case 30:
		p.salary_modifier(1000 * roll(1));
		return { ActionType::None };
	case 31:
		playerMove(p, true, 4);
		return { ActionType::DefaultEnd };
	case 32:
		p.reputation_modifier(-p.reputation / 2);
		return { ActionType::None };
	case 33:
		playerMove(p, true, 2);
		return { ActionType::DefaultEnd };
	case 34:
		p.money_modifier(1000 * roll(1));
		return { ActionType::None };
	case 35:
		p.cant_move_modifier(true);
		return { ActionType::None };
	case 36:
		p.cant_move_modifier(true);
		return { ActionType::None };
	case 37:
		playerMove(p, true, 4);
		return { ActionType::DefaultEnd };
	case 38:
		p.salary_modifier(-p.salary / 2);
		return { ActionType::None };
	case 39:
		p.cant_move_modifier(true);
		return { ActionType::None };
	case 40:
		p.money_modifier(1000 * roll(1));
		return { ActionType::None };
	case 41:
		p.money_modifier(-p.money / 2);
		return { ActionType::None };
	case 45:
		p.degree_modifier((DegreeType)event[0]);
		p.salary_modifier(2000);
		return { ActionType::None };
	default:
		return { ActionType::None };
	}
}

int TableInstance::playerScore(const Player& p) const {
	return
		p.money / 1000 * settings.getScorePerKMoney() +
		p.happiness * settings.getScorePerHappiness() +
		p.reputation * settings.getScorePerReputation();
}

void TableInstance::playerInit(Player& p) {
	p.position = MapPoint::getStart();
	p.degree = DegreeType::None;
	p.retireTimes = 0;
	p.careerInfo = Player::careerTemplate;
	p.money = settings.getStartMoney();
	p.happiness = settings.getStartHappiness();
	p.reputation = settings.getStartReputation();
	p.salary = settings.getStartSalary();
	p.doubleHappiness = false;
	p.careerSelect = false;
	p.cantMove = false;
}

void TableInstance::playerBankrupt(Player& p) {
	p.position = MapPoint::getStart();
	p.money = settings.getStartMoney();
	p.happiness = settings.getStartHappiness();
	p.reputation = settings.getStartReputation();
	p.salary = settings.getStartSalary();
	p.doubleHappiness = false;
	p.careerSelect = false;
	p.cantMove = false;
	sendState();
}

void TableInstance::playerMove(Player& p, bool leap, int step) {
	if (leap) {
		while (p.position != MapHelper::getPoint(step)) {
			if (p.careerSelect) {
				p.position_modifier(p.position->next_entry());
				p.career_select_modifier(false);
			}
			else {
				p.position_modifier(p.position->next());
			}

			if (p.doubleHappiness && p.position->nid == 31) {
				p.double_happiness_modifier(false);
			}

			this_thread::sleep_for(chrono::milliseconds(moveIntervalInMilliseconds));
		}
	}
	else {
		for (int i = 0; i < step; ++i) {
			if (p.careerSelect) {
				p.position_modifier(p.position->next_entry());
				p.career_select_modifier(false);
			}
			else {
				if (p.position->next()->exit && p.position->pointType == PointType::Career) {
					if (p.position->nid == 13) {
						while (true) {
							p.state == PlayerState::Robot ? oneTimeRequestRobot(p.pid, EVENT_GRADUATE) : oneTimeRequest(p.pid, EVENT_GRADUATE);
							if (checkEvents(EVENT_GRADUATE)) {
								handleEvents(EVENT_GRADUATE);
								break;
							}
							sendHint(HintType::ResponseError, { p.pid });
						}
					}
					else {
						CareerType type = CareerType::Sea;
						switch (p.position->nid)
						{
						case 19:
							type = CareerType::Moon; break;
						case 8:
							type = CareerType::Miner; break;
						case 11:
							type = CareerType::Movie; break;
						case 29:
							type = CareerType::Politic; break;
						case 32:
							type = CareerType::Sea; break;
						case 15:
							type = CareerType::Company; break;
						case 23:
							type = CareerType::Farmer; break;
						default: break;
						}
						p.career_increment(type);
						if (p[type] <= 3) drawCard(true, p.pid);
						if (p[type] == 3) p.retire_time_modifier(1);
					}
				}

				p.position_modifier(p.position->next());

				if (p.position == MapPoint::getStart()) {
					p.money_modifier(p.salary);
				}
			}

			if (p.doubleHappiness && p.position->nid == 31) {
				p.double_happiness_modifier(false);
			}

			this_thread::sleep_for(chrono::milliseconds(moveIntervalInMilliseconds));
		}
	}

	if (p.position == MapPoint::getHoliday()) {
		p.happiness_modifier(4);
	}
}

int TableInstance::playerWithDegree(DegreeType degree) const {
	auto it = find_if(players.begin(), players.end(), [degree](const auto& p) { return p.second.degree == degree; });
	return it == players.end() ? 0u : it->first;
}

void TableInstance::oneTimeRequest(int pid, int eid) {
	event.eventReset();

	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::PlayerRequest } },
		{ "D", { unordered_map<string, json> { { "E", { eid } } } } }
	} });

	auto tpStart = chrono::system_clock::now();

	while (!event) {
		this_thread::sleep_for(chrono::milliseconds(waitingIntervalInMilliseconds));
		
		auto tp = chrono::system_clock::now();
		
		if (players.at(pid).state == PlayerState::HumanLeft) {
			if (chrono::duration_cast<chrono::milliseconds>(tp - tpStart).count() > maximumWaitingTimeInMillisecondsForAutoPlayer) {
				eventSetDefault(pid, eid);
			}
		}

		if (chrono::duration_cast<chrono::milliseconds>(tp - tpStart).count() > maximumWaitingTimeInMilliseconds) {
			eventSetDefault(pid, eid);
			sendTimeout(pid);			
			players.at(pid).state = PlayerState::HumanLeft;
			send(pid, { unordered_map<string, json> {
				{ "PT", { (int)PacketTypeServer::PlayerSetAutoAcknowledge } },
				{ "AUTO", { true } }
			} });
		}
	}

	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::PlayerAcknowlege } }
	} });
}

void TableInstance::oneTimeRequestRobot(int pid, int eid) {
	event.eventReset();
	eventSetDefault(pid, eid);
}

void TableInstance::sendTimeout(int pid) const {
	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::PlayerTimeout } },
	} });
}

void TableInstance::sendState() const {
	json data(unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::TableSync } },
		{ "D", capture() }
	});

	for (auto it = players.cbegin(); it != players.cend(); ++it) {
		send(it->first, data);
	}
}

void TableInstance::sendHint(HintType hintType, initializer_list<int64_t> param) const {
	vector<json> params;
	for (const int64_t* p = param.begin(); p != param.end(); ++p) {
		params.emplace_back(*p);
	}

	json data(unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomHint } },
		{ "D", { unordered_map<string, json> { { "HT", { (int)hintType } }, { "LI", { params } } } } }
	});

	for (auto it = players.cbegin(); it != players.cend(); ++it) {
		send(it->first, data);
	}
}

void TableInstance::cardListInit() {
	for (const ChCard& card : CardDesk::chCards)
		chCardMap.emplace(card.cid, 0u);
	for (const ExCard& card : CardDesk::exCards)
		exCardMap.emplace(card.cid, 0u);
	currentExList.clear();
	currentChList.clear();
	discardExList.clear();
	discardChList.clear();
	for (auto it = chCardMap.cbegin(); it != chCardMap.cend(); ++it)
		currentChList.push_back(it->first);
	for (auto it = exCardMap.cbegin(); it != exCardMap.cend(); ++it)
		currentExList.push_back(it->first);
	cardListRoll(currentChList);
	cardListRoll(currentExList);
}

void TableInstance::cardListRoll(vector<int>& rolledList) {
	if (rolledList.empty()) return;
	random_device rd; mt19937 mt_rand(rd());
	for (size_t i = rolledList.size() - 1; i > 0; --i) {
		swap(rolledList[i], rolledList[mt_rand() % (i + 1)]);
	}
}

void TableInstance::drawCard(bool exp, int owner) {
	// 机器人暂时不抽卡 改为奖励1000元
	if (players.at(owner).state == PlayerState::Robot) {
		players.at(owner).money_modifier(1000);
		return;
	}

	if (exp)
	{
		if (currentExList.empty()) {
			sendHint(HintType::CardRunOutEx, { owner });
			return;
		}

		int cid = currentExList[0];
		currentExList.erase(currentExList.begin());
		exCardMap[cid] = owner;

		// 抽卡卡组最后一张卡片被取走后 将使用过的卡组打乱后作为新的抽卡卡组
		if (currentExList.empty())
		{
			currentExList = discardExList;
			cardListRoll(currentExList);
			discardExList.clear();
		}

		sendHint(HintType::DrawCardEx, { owner, cid });
	}
	else
	{
		if (currentChList.empty()) {
			sendHint(HintType::CardRunOutCh, { owner });
			return;
		}

		int cid = currentChList[0];
		currentChList.erase(currentChList.begin());
		chCardMap[cid] = owner;

		// 抽卡卡组最后一张卡片被取走后 将使用过的卡组打乱后作为新的抽卡卡组
		if (currentChList.empty())
		{
			currentChList = discardChList;
			cardListRoll(currentChList);
			discardChList.clear();
		}
		
		sendHint(HintType::DrawCardCh, { owner, cid });
	}
}

vector<json> TableInstance::collect(bool exp, int owner) const {
	vector<json> result;
	if (exp) {
		for (auto it = exCardMap.cbegin(); it != exCardMap.cend(); ++it) {
			if (it->second == owner) {
				result.emplace_back(it->first);
			}
		}
	}
	else {
		for (auto it = chCardMap.cbegin(); it != chCardMap.cend(); ++it) {
			if (it->second == owner) {
				result.emplace_back(it->first);
			}
		}
	}
	return result;
}

constexpr auto MONEY_FACTOR = 100;
constexpr auto NON_MONEY_FACTOR = 120;

/* Simple AI Module */
void TableInstance::eventSetDefault(int pid, int eid) {
	const Player& p = players.at(pid);

	switch (eid)
	{
	case 0:
		// Todo: Add AI Strategy for Cards.
		event.eventSet(vector<int> { 1 });
	case 5:
		if (p.money >= 12000 && 12 * NON_MONEY_FACTOR * settings.getScorePerReputation() >= 12 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 3 });
		else if (p.money >= 8000 && 8 * NON_MONEY_FACTOR * settings.getScorePerReputation() >= 8 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 2 });
		else if (p.money >= 4000 && 4 * NON_MONEY_FACTOR * settings.getScorePerReputation() >= 4 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 4 });
		break;
	case 6:
		if (p.degree == DegreeType::Science || p.careerInfo.at(CareerType::Moon) > 0 || p.money >= 5000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 7:
		if (p.money >= 16000 && 12 * NON_MONEY_FACTOR * settings.getScorePerHappiness() >= 16 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 3 });
		else if (p.money >= 8000 && 8 * NON_MONEY_FACTOR * settings.getScorePerHappiness() >= 8 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 2 });
		else if (p.money >= 2000 && 4 * NON_MONEY_FACTOR * settings.getScorePerHappiness() >= 2 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 4 });
		break;
	case 8:
		if (p.degree == DegreeType::Engineering || p.careerInfo.at(CareerType::Miner) > 0 || p.money >= 4000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 9:
		if (p.money >= 3000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 11:
		if (!(p.degree == DegreeType::None) || p.careerInfo.at(CareerType::Company) > 0 || p.money >= 500)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 12:
		if (p.money >= 6000 && 7 * NON_MONEY_FACTOR * settings.getScorePerHappiness() >= 6 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 2 });
		else if (p.money >= 3000 && 4 * NON_MONEY_FACTOR * settings.getScorePerHappiness() >= 3 * MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 3 });
		break;
	case 13:
		if (p.careerInfo.at(CareerType::Sea) > 0 || p.money >= 100)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 15:
		if (p.careerInfo.at(CareerType::Farmer) > 0 || p.money >= 1000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 16: {
		if (p.money >= p.salary && NON_MONEY_FACTOR * settings.getScorePerHappiness() >= MONEY_FACTOR * settings.getScorePerKMoney())
			event.eventSet(vector<int> { 2 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	}
	case 17: 
		if (!(p.degree == DegreeType::None) || p.money >= 500) 
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 18:
		if (p.careerInfo.at(CareerType::Movie) > 0 || p.money >= 1000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 20:
		if (p.degree == DegreeType::Law || p.careerInfo.at(CareerType::Politic) > 0 || p.money >= 3000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 26:
		if (p.money >= 5000)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 42:
		if (p.money >= p.salary / 2)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 45:
		if (playerWithDegree(DegreeType::Medicine) == 0u)
			event.eventSet(vector<int> { 2 });
		else if (playerWithDegree(DegreeType::Science) == 0u)
			event.eventSet(vector<int> { 4 });
		else if (playerWithDegree(DegreeType::Engineering) == 0u)
			event.eventSet(vector<int> { 3 });
		else if (playerWithDegree(DegreeType::Law) == 0u)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 5 });
		break;
	case 43:
		event.eventSet(vector<int> { 2 });
		break;
	case 44:
		if (p.money <= p.salary)
			event.eventSet(vector<int> { 1 });
		else
			event.eventSet(vector<int> { 2 });
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 10:
	case 14:
	case 19:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
	case 100:
	default:
		event.eventSet(vector<int> { 1 });
		break;
	}
}

Action::operator bool() const {
	return !(actionType == ActionType::None);
}

Action::Action(ActionType actionType) :
	actionType(actionType),
	specialEventId(0) {
}

Action::Action(int eventId) :
	actionType(ActionType::Extra),
	specialEventId(eventId) {
}
