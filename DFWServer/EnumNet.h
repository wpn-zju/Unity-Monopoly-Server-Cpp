#pragma once

enum struct PacketTypeServer;
enum struct PacketTypeClient;
enum struct RoomState;
enum struct ClientState;
enum struct PlayerState;
enum struct HintType;
enum struct CareerType;

enum struct PacketTypeServer {
    TableSync = 101,
    TableStart = 102,
    TableEnd = 103,
    PlayerRequest = 104,
    PlayerTimeout = 105,
    PlayerAcknowlege = 106,
    PlayerSetAutoAcknowledge = 107,
    RoomCreate = 201,
    RoomJoin = 202,
    RoomExit = 203,
    RoomHint = 204,
    RoomChat = 205,
    RoomInfo = 206,
    RoomSettings = 207,
    SystemConnect = 1001,
    SystemHeartbeat = 1002,
    SystemHint = 1003
};

enum struct PacketTypeClient {
    PlayerResponse = 101,
    PlayerSetAuto = 102,
    RoomCreate = 201,
    RoomJoin = 202,
    RoomRandomJoin = 203,
    RoomExit = 204,
    RoomSettings = 205,
    RoomPrepare = 206,
    RoomRobot = 207,
    RoomKick = 208,
    RoomChat = 209,
    RoomHost = 210,
    RoomSyncRequest = 211,
    SystemHeartbeat = 1001
};

enum struct RoomState {
    InRoom = 1,
    InGame = 2
};

enum struct ClientState {
    Host = 1,
    Ready = 2,
    NotReady = 3,
    Robot = 100
};

enum struct PlayerState {
    Human = 1,
    HumanLeft = 2,
    Robot = 3
};

enum struct HintType {
    Bankruptcy = 1,
    StartRound = 3,
    Roll = 4,
    UseChCard = 5,
    UseExCard = 6,
    Crash = 7,
    ResponseError = 9,
    Money = 10,
    Happiness = 11,
    Reputation = 12,
    Salary = 13,
    DoubleHappiness = 14,
    CareerSelect = 16,
    FinishCareer = 17,
    CantMove = 18,
    Retire = 20,
    DrawCardCh = 22,
    DrawCardEx = 23,
    CardRunOutCh = 24,
    CardRunOutEx = 25
};

enum struct CareerType {
    Farmer = 1,
    Company = 2,
    Sea = 3,
    Politic = 4,
    Movie = 5,
    Miner = 6,
    Moon = 7
};
