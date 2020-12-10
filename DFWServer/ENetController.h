#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include <chrono>
#include "enet.h"
#include "json.h"
#include "EnumNet.h"
#include "Room.h"

class ENetController {
public:
    void start();

    ENetController();
    ~ENetController();

private:
    ENetAddress address;
    ENetHost* server;

    std::unordered_map<ENetPeer*, int> connectMap;
    std::unordered_map<int, ENetPeer*> reverseConnectMap;
    std::unordered_map<int, int> clientRoomMap;
    std::unordered_map<int, Room> roomMap;

    void send(int, const json&);
    void receive();
    void confirmConnect(int);
    void handleEvent(int, const json&);
    void playerResponse(int, const json&);
    void playerSetAuto(int, const json&);
    void roomCreate(int, const json&);
    void roomJoin(int, const json&);
    void roomRandomJoin(int, const json&);
    void roomExit(int, const json&);
    void roomSettings(int, const json&);
    void roomPrepare(int, const json&);
    void roomRobot(int, const json&);
    void roomKick(int, const json&);
    void roomChat(int, const json&);
    void roomHost(int, const json&);
    void roomSync(int, const json&);
    void systemHeartbeat(int, const json&);
    void sendRoomInfo(const Room&);
    void sendRoomSettings(const Room&);
    json captureRoom(const Room&);
};
