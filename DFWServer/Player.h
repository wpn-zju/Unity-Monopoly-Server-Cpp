#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include "json.h"
#include "EnumNet.h"
#include "MapPoint.h"
#include "Utility.h"

enum struct DegreeType;

class Player {
public:
	PlayerState state;
	const int pid;	
	const MapPoint* position;
	int money;
	int happiness;
	int reputation;
	int salary;
	DegreeType degree;
	bool doubleHappiness;
	bool careerSelect;
	bool cantMove;
	int retireTimes;
	std::unordered_map<CareerType, int, EnumClassHash> careerInfo;

	void position_modifier(const MapPoint*);
	void degree_modifier(DegreeType);
	void retire_time_modifier(int);
	void career_increment(CareerType); 
	void money_modifier(int);
	void happiness_modifier(int);
	void reputation_modifier(int);
	void salary_modifier(int);
	void double_happiness_modifier(bool);
	void career_select_modifier(bool value);
	void cant_move_modifier(bool value);

	json capture() const;

	bool operator==(const int) const;
	bool operator==(const Player&) const;
	const int operator[](CareerType) const;

	Player(int, bool, std::function<void()>, std::function<void(HintType, std::initializer_list<int64_t>)>);
	Player(const Player&) = delete;
	Player(Player&&) = delete;
	Player& operator=(const Player&) = delete;
	Player& operator=(Player&&) = delete;
	~Player();

	static const std::unordered_map<CareerType, int, EnumClassHash> careerTemplate;

private:
	const std::function<void()> send_state;
	const std::function<void(HintType, std::initializer_list<int64_t>)> hint;
	std::unordered_map<std::string, json> captureCareer() const;
};

enum struct DegreeType {
	None = 0,
	Law = 1,
	Medicine = 2,
	Engineering = 3,
	Science = 4,
	Normal = 5
};
