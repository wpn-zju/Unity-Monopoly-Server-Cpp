#include "CardManager.h"

using namespace std;

const vector<ExCard> CardDesk::exCards = {
	{  0, 1 },
	{  1, 1 },
	{  2, 1 },
	{  3, 1 },
	{  4, 1 },
	{  5, 2 },
	{  6, 2 },
	{  7, 2 },
	{  8, 2 },
	{  9, 2 },
	{ 10, 3 },
	{ 11, 3 },
	{ 12, 3 },
	{ 13, 3 },
	{ 14, 3 },
	{ 15, 4 },
	{ 16, 4 },
	{ 17, 4 },
	{ 18, 4 },
	{ 19, 4 }
};

const vector<ChCard> CardDesk::chCards = {
	{  0, ChCardType::SpecialMoon },
	{  1, ChCardType::NormalSea},
	{  2, ChCardType::SpecialMovie},
	{  3, ChCardType::Holiday},
	{  4, ChCardType::NormalPolitic},
	{  5, ChCardType::NormalCompany},
	{  6, ChCardType::SpecialPolitic},
	{  7, ChCardType::NormalMoon},
	{  8, ChCardType::NormalFarmer},
	{  9, ChCardType::NormalCompany},
	{ 10, ChCardType::NormalStudy},
	{ 11, ChCardType::NormalMiner},
	{ 12, ChCardType::NormalFarmer},
	{ 13, ChCardType::NormalPolitic},
	{ 14, ChCardType::NormalSea},
	{ 15, ChCardType::NormalMovie},
	{ 16, ChCardType::NormalAny},
	{ 17, ChCardType::NormalStudy},
	{ 18, ChCardType::NormalMiner},
	{ 19, ChCardType::SpecialSea2X},
	{ 20, ChCardType::NormalMoon},
	{ 21, ChCardType::NormalFarmer},
	{ 22, ChCardType::NormalCompany},
	{ 23, ChCardType::NormalStudy},
	{ 24, ChCardType::SpecialMiner},
	{ 25, ChCardType::NormalMovie},
	{ 26, ChCardType::NormalAny},
	{ 27, ChCardType::Holiday}
};

ExCard::ExCard(int cid, int step) :
	cid(cid),
	step(step) {
}

ChCard::ChCard(int cid, ChCardType chCardType) :
	cid(cid),
	chCardType(chCardType) {
}
