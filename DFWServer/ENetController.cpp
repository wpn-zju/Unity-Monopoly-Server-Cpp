#include "ENetController.h"

#include <cstdio>
#include <thread>

constexpr auto ID_MAX = 65535;

using namespace std;

void ENetController::start() {
	thread lThread(&ENetController::receive, this);
	lThread.join();
}

void ENetController::send(int pid, const json& message) {
	if (!reverseConnectMap.count(pid)) return;
	string str = message.serialize();
	ENetPacket* packet = enet_packet_create(str.c_str(), str.length() + 1u, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(reverseConnectMap.at(pid), 0u, packet);
	enet_host_flush(server);
	enet_packet_dispose(packet);
}

void ENetController::receive() {
	while (true) {
		ENetEvent event;
		while (enet_host_service(server, &event, 10) > 0) {
			union {
				struct { uint8_t s_b1, s_b2, s_b3, s_b4; } S_un_b;
				struct { uint16_t s_w1, s_w2; } S_un_w;
				uint32_t S_addr;
			} S_un;
			S_un.S_addr = event.peer->address.ipv4.ip.s_addr;
			switch (event.type) {
			case ENET_EVENT_TYPE_NONE:
				printf("Received NONE-Type packet from %u.%u.%u.%u:%u.\n",
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					event.peer->address.port);
				break;
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %u.%u.%u.%u:%u.\n",
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					event.peer->address.port);
				event.peer->data = (void*)"Client Information";
				{
					int pid = event.peer->eventData;
					if (connectMap.count(event.peer)) {
						printf("Peer already in memory. Pid: %d.\n", connectMap.at(event.peer));
						pid = connectMap.at(event.peer);
					}
					else {
						if (reverseConnectMap.count(pid)) {
							if (reverseConnectMap.at(pid) == event.peer) {
								// Todo: Remove this conditional branch
								printf("Peer Equal\n");
							}
							else {
								printf("Peer Kicked\n");
								enet_peer_disconnect(reverseConnectMap.at(pid), 1u);
							}
						}
						else {
							random_device rd; std::mt19937 mt_rand(rd());
							pid = mt_rand() % ID_MAX;
						}
					}
					reverseConnectMap[pid] = event.peer;
					connectMap[event.peer] = pid;
					confirmConnect(pid);
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet of length %u was received from from %u.%u.%u.%u:%u on channel %u.\n",
					event.packet->dataLength,
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					event.peer->address.port,
					event.channelID);
				if (connectMap.count(event.peer))
					handleEvent(connectMap.at(event.peer), json::create((const char*)event.packet->data));
				else
					printf("Warning - Receive packet from a non-connected peer!\n");
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("Disconnected from %u.%u.%u.%u:%u.\n",
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					event.peer->address.port);
				event.peer->data = nullptr;
				{
					if (connectMap.count(event.peer)) {
						int pid = connectMap.at(event.peer);
						if (reverseConnectMap.at(pid) == event.peer)
							reverseConnectMap.erase(connectMap.at(event.peer));
						connectMap.erase(event.peer);
					}
					else {
						printf("Warning - Receive disconnect request from a non-connected peer!\n");
					}
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				printf("Timeout from %u.%u.%u.%u:%u.\n",
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					event.peer->address.port);
				event.peer->data = nullptr;
				{
					if (connectMap.count(event.peer)) {
						int pid = connectMap.at(event.peer);
						if (reverseConnectMap.at(pid) == event.peer)
							reverseConnectMap.erase(connectMap.at(event.peer));
						connectMap.erase(event.peer);
					}
					else {
						printf("Warning - Timeout from a non-connected peer!\n");
					}
				}
				break;
			}
			for (auto it = connectMap.cbegin(); it != connectMap.cend(); ++it) {
				union {
					struct { uint8_t s_b1, s_b2, s_b3, s_b4; } S_un_b;
					struct { uint16_t s_w1, s_w2; } S_un_w;
					uint32_t S_addr;
				} S_un;
				S_un.S_addr = it->first->address.ipv4.ip.s_addr;
				printf("CM - %u.%u.%u.%u:%u: %d\n", 
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2, 
					S_un.S_un_b.s_b3, 
					S_un.S_un_b.s_b4,
					it->first->address.port,
					it->second);
			}
			for (auto it = reverseConnectMap.cbegin(); it != reverseConnectMap.cend(); ++it) {
				union {
					struct { uint8_t s_b1, s_b2, s_b3, s_b4; } S_un_b;
					struct { uint16_t s_w1, s_w2; } S_un_w;
					uint32_t S_addr;
				} S_un;
				S_un.S_addr = it->second->address.ipv4.ip.s_addr;
				printf("RM - %d: %u.%u.%u.%u:%u\n",
					it->first,
					S_un.S_un_b.s_b1,
					S_un.S_un_b.s_b2,
					S_un.S_un_b.s_b3,
					S_un.S_un_b.s_b4,
					it->second->address.port);
			}
		}
	}
}

void ENetController::confirmConnect(int pid) {
	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::SystemConnect } },
		{ "D", { unordered_map<string, json> { { "PID", { pid } } } } }
	} });
	printf("Player %d Connected to Server\n", pid);
	if (!clientRoomMap.count(pid)) return;
	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomJoin } },
		{ "D", captureRoom(rRoom) }
	} });
	sendRoomSettings(rRoom);
	if (rRoom.state == RoomState::InGame) {
		send(pid, { unordered_map<string, json> {
			{ "PT", { (int)PacketTypeServer::TableStart } },
			{ "D", rRoom.instance.capture() }
		} });
	}
}

void ENetController::handleEvent(int pid, const json& data) {
	switch ((PacketTypeClient)data["PT"].get_int())
	{
	case PacketTypeClient::PlayerResponse:
		playerResponse(pid, data); break;
	case PacketTypeClient::PlayerSetAuto:
		playerSetAuto(pid, data); break;
	case PacketTypeClient::RoomCreate:
		roomCreate(pid, data); break;
	case PacketTypeClient::RoomJoin:
		roomJoin(pid, data); break;
	case PacketTypeClient::RoomRandomJoin:
		roomRandomJoin(pid, data); break;
	case PacketTypeClient::RoomExit:
		roomExit(pid, data); break;
	case PacketTypeClient::RoomSettings:
		roomSettings(pid, data); break;
	case PacketTypeClient::RoomPrepare:
		roomPrepare(pid, data); break;
	case PacketTypeClient::RoomRobot:
		roomRobot(pid, data); break;
	case PacketTypeClient::RoomKick:
		roomKick(pid, data); break;
	case PacketTypeClient::RoomChat:
		roomChat(pid, data); break;
	case PacketTypeClient::RoomHost:
		roomHost(pid, data); break;
	case PacketTypeClient::RoomSyncRequest:
		roomSync(pid, data); break;
	case PacketTypeClient::SystemHeartbeat:
		systemHeartbeat(pid, data); break;
	default: break;
	}
}

void ENetController::playerResponse(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));

	if (rRoom.state == RoomState::InGame && rRoom.instance.active == pid) {
		rRoom.instance.event.eventSet(packet["R"].get_vector());
	}
}

void ENetController::playerSetAuto(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;
	
	Room& rRoom = roomMap.at(clientRoomMap.at(pid));

	if (rRoom.state == RoomState::InGame) {
		if (rRoom.instance.players.at(pid).state == PlayerState::Human) {
			rRoom.instance.players.at(pid).state = PlayerState::HumanLeft;

			send(pid, { unordered_map<string, json> {
				{ "PT", { (int)PacketTypeServer::PlayerSetAutoAcknowledge } },
				{ "AUTO", { true } }
			} });
		}
		else if (rRoom.instance.players.at(pid).state == PlayerState::HumanLeft) {
			rRoom.instance.players.at(pid).state = PlayerState::Human;
			
			send(pid, { unordered_map<string, json> {
				{ "PT", { (int)PacketTypeServer::PlayerSetAutoAcknowledge } },
				{ "AUTO", { false } }
			} });
		}
	}
}

void ENetController::roomCreate(int pid, const json& packet) {
	if (clientRoomMap.count(pid)) roomExit(pid, packet);

	random_device rd; mt19937 mt_rand(rd());
	int rid = mt_rand() % ID_MAX;
	roomMap.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(rid),
		std::forward_as_tuple(rid, pid,
			[this](int pid, const json& data) { send(pid, data); }));
	Room& nRoom = roomMap.at(rid);
	clientRoomMap[pid] = rid;

	send(pid, { unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomCreate } },
		{ "D", captureRoom(nRoom) }
	} });

	sendRoomInfo(nRoom);

	sendRoomSettings(nRoom);
}

void ENetController::roomJoin(int pid, const json& packet) {
	if (!roomMap.count(packet["RID"].get_int())) return;
	if (clientRoomMap.count(pid)) roomExit(pid, packet);

	Room& rRoom = roomMap.at(packet["RID"].get_int());
	if (rRoom.clients.size() == 4) {

	}
	else {
		clientRoomMap[pid] = rRoom.rid;
		rRoom.clients.emplace(pid, ClientState::NotReady);

		send(pid, { unordered_map<string, json> {
			{ "PT", { (int)PacketTypeServer::RoomJoin } },
			{ "D", captureRoom(rRoom) }
		} });

		sendRoomInfo(rRoom);

		sendRoomSettings(rRoom);
	}
}

void ENetController::roomRandomJoin(int pid, const json& packet) {

}

void ENetController::roomExit(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.host == pid) {
		json data(unordered_map<string, json> {
			{ "PT", { (int)PacketTypeServer::RoomExit } },
			{ "CC", { true } },
			{ "KK", { false } },
			{ "D", captureRoom(rRoom) }
		});

		for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
			clientRoomMap.erase(it->first);

			if (it->second != ClientState::Robot) {
				send(it->first, data);
			}
		}

		roomMap.erase(rRoom.rid);
	}
	else {
		if (rRoom.clients.count(pid)) {
			clientRoomMap.erase(pid);
			rRoom.clients.erase(pid);

			send(pid, { unordered_map<string, json> {
				{ "PT", { (int)PacketTypeServer::RoomExit } },
				{ "CC", { false } },
				{ "KK", { false } },
				{ "D", captureRoom(rRoom) }
			} });

			sendRoomInfo(rRoom);
		}
	}
}

void ENetController::roomSettings(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.host == pid) {
		rRoom.instance.settings.update((EnumSettings)packet["S"].get_int(), packet["V"].get_bool());

		sendRoomSettings(rRoom);
	}
}

void ENetController::roomPrepare(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.state == RoomState::InRoom && rRoom.host == pid) {
		bool couldStart = true;

		for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
			if (it->second == ClientState::NotReady) {
				couldStart = false;
			}
		}

		if (!couldStart) return;

		rRoom.launch();

		json data(unordered_map<string, json> {
			{ "PT", { (int)PacketTypeServer::TableStart } },
			{ "D", rRoom.instance.capture() }
		});

		for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
			if (it->second != ClientState::Robot) {
				send(it->first, data);
			}
		}
	}
	else {
		if (rRoom.clients.count(pid)) {
			if (rRoom.clients.at(pid) == ClientState::Ready) {
				rRoom.clients.at(pid) = ClientState::NotReady;
				sendRoomInfo(rRoom);
			}
			else if (rRoom.clients.at(pid) == ClientState::NotReady) {
				rRoom.clients.at(pid) = ClientState::Ready;
				sendRoomInfo(rRoom);
			}
		}
	}
}

void ENetController::roomRobot(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.state == RoomState::InRoom && rRoom.host == pid) {
		if (rRoom.clients.size() == 4) return;
		random_device rd; mt19937 mt_rand(rd());
		rRoom.clients.emplace(mt_rand() % ID_MAX, ClientState::Robot);
		sendRoomInfo(rRoom);
	}
}

void ENetController::roomKick(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.state == RoomState::InRoom && rRoom.host == pid) {
		int kid = packet["KID"].get_int();

		if (kid != rRoom.host) {
			if (rRoom.clients.count(kid)) {
				clientRoomMap.erase(kid);
				rRoom.clients.erase(kid);
				send(kid, { unordered_map<string, json> {
					{ "PT", { (int)PacketTypeServer::RoomExit } },
					{ "CC", { false } },
					{ "KK", { true } },
					{ "D", captureRoom(rRoom) }
				} });
				sendRoomInfo(rRoom);
			}
		}
	}
}

void ENetController::roomChat(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.state == RoomState::InRoom || rRoom.state == RoomState::InGame) {
		json data(unordered_map<string, json> {
			{ "PT", { (int)PacketTypeServer::RoomChat } },
			{ "D", { unordered_map<string, json>{ { "P", { pid } }, { "MSG", packet["MSG"] } } } }
		});

		for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
			if (it->second != ClientState::Robot) {
				send(it->first, data);
			}
		}
	}
}

void ENetController::roomHost(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	if (rRoom.state == RoomState::InRoom && rRoom.host == pid) {
		int hid = packet["HID"].get_int();

		if (hid != rRoom.host) {
			if (rRoom.clients.count(hid) && !(rRoom.clients[hid] == ClientState::Robot)) {
				rRoom.clients[rRoom.host] = ClientState::NotReady;
				rRoom.clients[hid] = ClientState::Host;
				rRoom.host = hid;
				sendRoomInfo(rRoom);
			}
		}
	}
}

void ENetController::roomSync(int pid, const json& packet) {
	if (!clientRoomMap.count(pid)) return;

	Room& rRoom = roomMap.at(clientRoomMap.at(pid));
	send(pid, unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomInfo } },
		{ "D", captureRoom(rRoom) }
	});
}

void ENetController::systemHeartbeat(int pid, const json& packet) {

}

void ENetController::sendRoomInfo(const Room& rRoom) {
	json data(unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomInfo } },
		{ "D", captureRoom(rRoom) }
	});

	for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
		if (it->second != ClientState::Robot) {
			send(it->first, data);
		}
	}
}

void ENetController::sendRoomSettings(const Room& rRoom) {
	json data(unordered_map<string, json> {
		{ "PT", { (int)PacketTypeServer::RoomSettings } },
		{ "D", rRoom.instance.settings.capture() }
	});

	for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
		if (it->second != ClientState::Robot) {
			send(it->first, data);
		}
	}
}

json ENetController::captureRoom(const Room& rRoom) {
	unordered_map<string, json> players;
	for (auto it = rRoom.clients.cbegin(); it != rRoom.clients.cend(); ++it) {
		players[to_string(it->first)] = { unordered_map<string, json> {
			{ "STT", { (int)it->second } },
			{ "PNG", { it->second == ClientState::Robot ? 0u : reverseConnectMap.at(it->first)->roundTripTime } }
		} };
	}

	return {
		unordered_map<string, json> {
			{ "RID", { rRoom.rid } },
			{ "H", { rRoom.host } },
			{ "C", { move(players) } }
		}
	};
}

ENetController::ENetController() {
	address.ipv6 = ENET_HOST_ANY;
	address.port = 58889;

	server = enet_host_create(&address, 32, 2, 0, 0, ENET_HOST_BUFFER_SIZE_MAX);

	if (server == nullptr) {
		printf("Error occured while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
	}
}

ENetController::~ENetController() {
	enet_host_destroy(server);
	server = nullptr;
}
