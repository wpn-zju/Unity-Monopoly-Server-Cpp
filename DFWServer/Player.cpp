#include "Player.h"

#include <algorithm>
#include "json.h"
#include "MapHelper.h"

using namespace std;

const unordered_map<CareerType, int, EnumClassHash> Player::careerTemplate = {
	{ CareerType::Company, 0 },
	{ CareerType::Farmer, 0 },
	{ CareerType::Miner, 0 },
	{ CareerType::Moon, 0 },
	{ CareerType::Movie, 0 },
	{ CareerType::Politic, 0 },
	{ CareerType::Sea, 0 },
};

bool Player::operator==(const int pid) const {
	return this->pid == pid;
}

bool Player::operator==(const Player& that) const {
	return this->pid == that.pid;
}

/* Read Only */
const int Player::operator[](CareerType careerType) const {
	return careerInfo.at(careerType);
}

void Player::position_modifier(const MapPoint* to) {
	position = to;
	send_state();
}

void Player::degree_modifier(DegreeType d) {
	degree = d;
	send_state();
}

void Player::retire_time_modifier(int value) {
	int ori = retireTimes;
	int now = retireTimes + value;
	now = max(now, 0);
	retireTimes = now;
	if (ori != now) {
		send_state();
		hint(HintType::Retire, { pid, ori, now });
	}
}

void Player::career_increment(CareerType career) {
	++careerInfo[career];
	send_state();
	hint(HintType::FinishCareer, { pid, (int)career });
}

void Player::money_modifier(int value) {
	int ori = money;
	int now = money + value;
	now = now - now % 10;
	money = now;
	if (ori != now) {
		send_state();
		hint(HintType::Money, { pid, ori, now });
	}
}

void Player::happiness_modifier(int value) {
	int ori = happiness;
	int now = happiness + value;
	now = max(now, 0);
	happiness = now;
	if (ori != now) {
		send_state();
		hint(HintType::Happiness, { pid, ori, now });
	}
}

void Player::reputation_modifier(int value) {
	int ori = reputation;
	int now = reputation + value;
	now = max(now, 0);
	reputation = now;
	if (ori != now) {
		send_state();
		hint(HintType::Reputation, { pid, ori, now });
	}
}

void Player::salary_modifier(int value) {
	int ori = salary;
	int now = salary + value;
	now = max(now, 0);
	now = now - now % 50;
	salary = now;
	if (ori != now) {
		send_state();
		hint(HintType::Salary, { pid, ori, now });
	}
}

void Player::double_happiness_modifier(bool value) {
	bool ori = doubleHappiness;
	bool now = value;
	doubleHappiness = now;
	if (ori != now) {
		send_state();
		hint(HintType::DoubleHappiness, { pid, ori, now });
	}
}

void Player::career_select_modifier(bool value) {
	bool ori = careerSelect;
	bool now = value;
	careerSelect = now;
	send_state();
	if (ori != now) {
		send_state();
		hint(HintType::CareerSelect, { pid, ori, now });
	}
}

void Player::cant_move_modifier(bool value) {
	bool ori = cantMove;
	bool now = value;
	cantMove = now;
	if (ori != now) {
		send_state();
		hint(HintType::CantMove, { pid, ori, now });
	}
}

json Player::capture() const {
	return {
		unordered_map<string, json>{
		{ "ID", { pid } },
		{ "STT", { (int)state } },
		{ "P", { position->pid } },
		{ "M", { money } },
		{ "H", { happiness } },
		{ "R", { reputation } },
		{ "S", { salary } },
		{ "DH", { doubleHappiness } },
		{ "CS", { careerSelect } },
		{ "CM", { cantMove } },
		{ "CI", { captureCareer() } } } };
}

unordered_map<string, json> Player::captureCareer() const {
	return {
		{ "D", { (int)degree } },
		{ "RT", { retireTimes } },
		{ "FT", { careerInfo.at(CareerType::Farmer) } },
		{ "CT", { careerInfo.at(CareerType::Company) } },
		{ "ST", { careerInfo.at(CareerType::Sea) } },
		{ "PT", { careerInfo.at(CareerType::Politic) } },
		{ "VT", { careerInfo.at(CareerType::Movie) } },
		{ "NT", { careerInfo.at(CareerType::Miner) } },
		{ "OT", { careerInfo.at(CareerType::Moon) } }
	};
}

Player::Player(int pid, bool robot, function<void()> send_state, function<void(HintType, std::initializer_list<int64_t>)> hint) :
	state(robot ? PlayerState::Robot : PlayerState::Human),
	pid(pid),
	position(MapPoint::getStart()),
	money(0),
	happiness(0),
	reputation(0),
	salary(0),
	degree(DegreeType::None),
	doubleHappiness(false),
	careerSelect(false),
	cantMove(false),
	retireTimes(0),
	careerInfo(careerTemplate),
	send_state(send_state),
	hint(hint) {
}

Player::~Player() {

}
