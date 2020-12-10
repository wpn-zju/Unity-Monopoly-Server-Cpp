#include "Room.h"

using namespace std;

void Room::launch() {
	instance.init(clients);
	thread th(&Room::runInstance, this);
	th.detach();
}

void Room::runInstance() {
	state = RoomState::InGame;
	instance.start();
	quitInstance();
}

void Room::quitInstance() {
	state = RoomState::InRoom;
	
	for (auto it = clients.begin(); it != clients.end(); ++it) {
		if (it->second == ClientState::Ready) {
			it->second = ClientState::NotReady;
		}
	}
}

Room::Room(int rid, int host, function<void(int, const json&)> send) :
	rid(rid),
	host(host),
	state(RoomState::InRoom),
	clients(),
	instance(send),
	send(send) {
	clients.emplace(host, ClientState::Host);
}

Room::~Room() {

}
