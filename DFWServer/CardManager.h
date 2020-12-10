#pragma once

#include <vector>

struct ExCard;
struct ChCard;
enum struct ChCardType;

class CardDesk {
public:
	static const std::vector<ExCard> exCards;
	static const std::vector<ChCard> chCards;

	CardDesk() = delete;
};

struct ExCard {
public:
	const int cid;
	const int step;

	ExCard(int, int);
};

struct ChCard {
public:
	const int cid;
	const ChCardType chCardType;

	ChCard(int, ChCardType);
};

enum struct ChCardType {
	NormalCompany = 0,
	NormalMoon = 1,
	NormalPolitic = 2,
	NormalMovie = 3,
	NormalMiner = 4,
	NormalFarmer = 5,
	NormalSea = 6,
	NormalStudy = 7,
	NormalAny = 8,
	Holiday = 9,
	SpecialMiner = 10,
	SpecialMovie = 11,
	SpecialMoon = 12,
	SpecialPolitic = 13,
	SpecialSea2X = 14,
};
