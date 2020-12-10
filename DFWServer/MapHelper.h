#pragma once

#include <unordered_map>
#include "MapPoint.h"

class MapHelper {
public:
	friend class MapPoint;

	static void mapSetup();
	static void mapCleanup();
	static const MapPoint* getPoint(int);

	MapHelper() = delete;
	MapHelper(const MapHelper& that) = delete;
	MapHelper(MapHelper&& that) = delete;
	MapHelper& operator=(const MapHelper&) = delete;
	MapHelper& operator=(MapHelper&&) = delete;
	~MapHelper() = delete;

private:
	static std::unordered_map<int, MapPoint*> mapDic;
};
