#pragma once

#include <unordered_map>
#include <functional>
#include <thread>
#include "json.h"
#include "EnumNet.h"
#include "GameInstance.h"

class Room {
public:	
	int rid;
	int host;
	RoomState state;
	std::unordered_map<int, ClientState> clients;
	TableInstance instance;

	void launch();

	Room(int, int, const std::function<void(int, const json&)>);
	Room(const Room&) = delete;
	Room(Room&&) = delete;
	Room& operator=(const Room&) = delete;
	Room& operator=(Room&&) = delete;
	~Room();

private:
	void runInstance();
	void quitInstance();
	const std::function<void(int, const json&)> send;
};
