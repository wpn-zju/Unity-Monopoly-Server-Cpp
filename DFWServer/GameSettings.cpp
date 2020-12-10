#include "GameSettings.h"

#include "json.h"

using namespace std;

const unordered_map<EnumSettings, Attribute, EnumClassHash> Attribute::attributeMap = {
	{ EnumSettings::Speed,				{ "GS",		1,		4,		1,		2 } },
	{ EnumSettings::WaitTime,			{ "WT",		10,		60,		10,		30 } },
	{ EnumSettings::StartMoney,			{ "SM",		2000,	20000,	1000,	2000 } },
	{ EnumSettings::StartHappiness,		{ "SH",		0,		10,		1,		0 } },
	{ EnumSettings::StartReputation,	{ "SR",		0,		10,		1,		0 } },
	{ EnumSettings::StartSalary,		{ "SS",		0,		5000,	1000,	1000 } },
	{ EnumSettings::ScorePerKMoney,		{ "SCKM",	1,		5,		1,		1 } },
	{ EnumSettings::ScorePerHappiness,	{ "SCH",	1,		5,		1,		1 } },
	{ EnumSettings::ScorePerReputation, { "SCR",	1,		5,		1,		1 } },
	{ EnumSettings::ScoreTarget,		{ "SCT",	60,		300,	10,		100 } }
};

int TableSettings::getSpeed() const {
	return settings.at(EnumSettings::Speed).value;
}

int TableSettings::getWaitTime() const {
	return settings.at(EnumSettings::WaitTime).value;
}

int TableSettings::getStartMoney() const {
	return settings.at(EnumSettings::StartMoney).value;
}

int TableSettings::getStartHappiness() const {
	return settings.at(EnumSettings::StartHappiness).value;
}

int TableSettings::getStartReputation() const {
	return settings.at(EnumSettings::StartReputation).value;
}

int TableSettings::getStartSalary() const {
	return settings.at(EnumSettings::StartSalary).value;
}

int TableSettings::getScorePerKMoney() const {
	return settings.at(EnumSettings::ScorePerKMoney).value;
}

int TableSettings::getScorePerHappiness() const {
	return settings.at(EnumSettings::ScorePerHappiness).value;
}

int TableSettings::getScorePerReputation() const {
	return settings.at(EnumSettings::ScorePerReputation).value;
}

int TableSettings::getScoreTarget() const {
	return settings.at(EnumSettings::ScoreTarget).value;
}

void TableSettings::setSpeed(int value) {
	settings.at(EnumSettings::Speed).value = clamp(value, 1, 4);
}

void TableSettings::setWaitTime(int value) {
	settings.at(EnumSettings::WaitTime).value = clamp(value, 10, 60);
}

void TableSettings::setStartMoney(int value) {
	settings.at(EnumSettings::StartMoney).value = clamp(value, 2000, 20000);
}

void TableSettings::setStartHappiness(int value) {
	settings.at(EnumSettings::StartHappiness).value = clamp(value, 0, 10);
}

void TableSettings::setStartReputation(int value) {
	settings.at(EnumSettings::StartReputation).value = clamp(value, 0, 10);
}

void TableSettings::setStartSalary(int value) {
	settings.at(EnumSettings::StartSalary).value = clamp(value, 0, 5000);
}

void TableSettings::setScorePerKMoney(int value) {
	settings.at(EnumSettings::ScorePerKMoney).value = clamp(value, 1, 5);
}

void TableSettings::setScorePerHappiness(int value) {
	settings.at(EnumSettings::ScorePerHappiness).value = clamp(value, 1, 5);
}

void TableSettings::setScorePerReputation(int value) {
	settings.at(EnumSettings::ScorePerReputation).value = clamp(value, 1, 5);
}

void TableSettings::setScoreTarget(int value) {
	settings.at(EnumSettings::ScoreTarget).value = clamp(value, 60, 300);
}

void TableSettings::update(EnumSettings key, bool inc) {
	settings.at(key).value = inc ?
		clamp(settings.at(key).value + settings.at(key).interval, settings.at(key).minimum, settings.at(key).maximum) :
		clamp(settings.at(key).value - settings.at(key).interval, settings.at(key).minimum, settings.at(key).maximum);
}

json TableSettings::capture() const {
	unordered_map<string, json> data;
	for (auto it = settings.cbegin(); it != settings.cend(); ++it)
		data[it->second.abbreviation] = { it->second.value };
	return { move(data) };
}

TableSettings::TableSettings() : settings(Attribute::attributeMap) {

}

TableSettings::~TableSettings() {

}

Attribute::Attribute(string&& abbre, int minimum, int maximum, int interval, int value) :
	abbreviation(abbre),
	minimum(minimum),
	maximum(maximum),
	interval(interval),
	value(value) {
}
