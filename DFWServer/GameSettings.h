#pragma once

#include <string>
#include <unordered_map>
#include "json.h"
#include "Utility.h"

enum struct EnumSettings;

struct Attribute {
public:
	static const std::unordered_map<EnumSettings, Attribute, EnumClassHash> attributeMap;

	std::string abbreviation;
	int minimum;
	int maximum;
	int interval;
	int value;

	Attribute(std::string&&, int, int, int, int);
};

class TableSettings {
private:
	std::unordered_map<EnumSettings, Attribute, EnumClassHash> settings;

public:
	int getSpeed() const;
	int getWaitTime() const;
	int getStartMoney() const;
	int getStartHappiness() const;
	int getStartReputation() const;
	int getStartSalary() const;
	int getScorePerKMoney() const;
	int getScorePerHappiness() const;
	int getScorePerReputation() const;
	int getScoreTarget() const;

	void update(EnumSettings, bool);

	// Deprecated - Use GameSettings::update(EnumSettings, bool) instead.
	void setSpeed(int value);
	void setWaitTime(int value);
	void setStartMoney(int value);
	void setStartHappiness(int value);
	void setStartReputation(int value);
	void setStartSalary(int value);
	void setScorePerKMoney(int value);
	void setScorePerHappiness(int value);
	void setScorePerReputation(int value);
	void setScoreTarget(int value);

	json capture() const;

	TableSettings();
	~TableSettings();
};

enum struct EnumSettings {
	Speed = 1,
	WaitTime = 2,
	StartMoney = 3,
	StartHappiness = 4,
	StartReputation = 5,
	StartSalary = 6,
	ScorePerKMoney = 7,
	ScorePerHappiness = 8,
	ScorePerReputation = 9,
	ScoreTarget = 10
};
