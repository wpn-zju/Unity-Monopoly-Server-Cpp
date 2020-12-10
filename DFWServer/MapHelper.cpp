#include "MapHelper.h"

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include "Utility.h"

using namespace std;

unordered_map<int, MapPoint*> MapHelper::mapDic;

void MapHelper::mapSetup() {
	mapCleanup();
	
	ifstream mapData("Map.txt");
	string data;
	string line;
	if (mapData.is_open()) {
		while (getline(mapData, line)) {
			data += line + '\n';
		}
		mapData.close();
	}

	data.erase(remove(data.begin(), data.end(), '\r'), data.end());

	vector<string> vecStr = split(data, '\n');
	
	vecStr.erase(vecStr.begin());

	for (string& str : vecStr) MapPoint::addEntry(str);

	printf("MapData Loaded!\n");

	return;
}

void MapHelper::mapCleanup() {
	for (auto it = mapDic.begin(); it != mapDic.end(); ++it) {
		delete(it->second);
	}
	mapDic.clear();
}

const MapPoint* MapHelper::getPoint(int pid) {
	return mapDic[pid];
}
