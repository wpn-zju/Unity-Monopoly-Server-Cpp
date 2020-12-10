#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include "json.h"
#include "EnumNet.h"
#include "GameSettings.h"
#include "CardManager.h"
#include "Event.h"
#include "Player.h"

struct Action;
enum struct ActionType;

class TableInstance {
public:	
	// Table Settings
	TableSettings settings;

	// Response Event
	Event event;

	// Player - add send state hook for these 2 members.
	std::unordered_map<int, Player> players;
	int active;

	// Cards
	std::vector<int> currentChList;
	std::vector<int> currentExList;
	std::vector<int> discardChList;
	std::vector<int> discardExList;
	std::unordered_map<int, int> chCardMap;
	std::unordered_map<int, int> exCardMap;

	// Public Calls
	void init(const std::unordered_map<int, ClientState>&);
	void start();
	json capture() const;

	TableInstance(const std::function<void(int, const json&)>);
	TableInstance(const TableInstance&) = delete;
	TableInstance(TableInstance&&) = delete;
	TableInstance& operator=(const TableInstance&) = delete;
	TableInstance& operator=(TableInstance&&) = delete;
	~TableInstance();

private:
	int maximumWaitingTimeInMillisecondsForAutoPlayer;
	int maximumWaitingTimeInMilliseconds;
	int waitingIntervalInMilliseconds;
	int moveIntervalInMilliseconds;

	// Player
	int playerScore(const Player&) const;
	int playerWithDegree(DegreeType) const;
	void playerInit(Player&);
	void playerMove(Player&, bool, int);
	void playerBankrupt(Player&);

	// Player Hook
	void setActivePlayer(int);

	// Main
	bool checkGameOver() const;
	void checkBankruptcy();
	void terminate();
	void cycle();
	void action();
	void crash();

	// Roll
	int roll(int) const;

	// Card
	void cardListInit();
	void cardListRoll(std::vector<int>& rolledList);
	void drawCard(bool exp, int owner);
	std::vector<json> collect(bool exp, int owner) const;

	// Event
	int getEventId(Action, const MapPoint*) const;
	bool checkEvents(Action) const;
	Action request(Action);
	Action handleEvents(Action);
	void eventSetDefault(int, int);

	// Interaction
	void oneTimeRequest(int, int);
	void oneTimeRequestRobot(int, int);
	void sendTimeout(int) const;
	void sendState() const;
	void sendHint(HintType, std::initializer_list<int64_t>) const;

	const std::function<void(int, const json&)> send;
};

struct Action {
public:
	ActionType actionType;
	int specialEventId;

	operator bool() const;

	Action(int);
	Action(ActionType);
};

enum struct ActionType {
	None,
	DefaultStart,
	DefaultEnd,
	Extra
};
