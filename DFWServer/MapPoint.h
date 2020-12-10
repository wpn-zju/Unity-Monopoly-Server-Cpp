#pragma once

#include <vector>
#include <string>

enum struct PointType;

class MapPoint {
public:
	static int addEntry(const std::string&);

	// Points
	static const MapPoint* getStart();
	static const MapPoint* getHospital();
	static const MapPoint* getHoliday();
	static const MapPoint* getBench();

	const bool entry;
	const bool exit;

	const int pid;
	const int nid;
	const int enid;

	const PointType pointType;

	const std::string title;
	const std::string note;

	const int eid_start;
	const int eid_end;
	const std::vector<int> params;

	// Support Chaining
	const MapPoint* next() const;
	const MapPoint* next_entry() const;

	bool operator==(const int) const;
	bool operator==(const MapPoint&) const;

	MapPoint(const MapPoint&) = delete;
	MapPoint(MapPoint&&) = delete;
	MapPoint& operator=(const MapPoint&) = delete;
	MapPoint& operator=(MapPoint&&) = delete;
	~MapPoint();

private:
	MapPoint(const std::vector<std::string>&);
};

enum struct PointType {
	Corner = 0,
	Normal = 1,
	Career = 2,
};
