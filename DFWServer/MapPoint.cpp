#include "MapPoint.h"

#include "MapHelper.h"
#include "Utility.h"

using namespace std;

vector<int> getParams(const string& data) {
	vector<int> result;
	vector<string> vec = split(data, '/');
	for (string& str : vec) {
		result.push_back(stoi(str));
	}
	return result;
}

int MapPoint::addEntry(const string& source) {
	vector<string> vecStr = split(source, '\t');
	int pid = stoi(vecStr[0]);
	MapHelper::mapDic.insert({ pid, new MapPoint(vecStr) });
	return pid;
}

const MapPoint* MapPoint::getStart() {
	return MapHelper::getPoint(1);
}

const MapPoint* MapPoint::getHospital() {
	return MapHelper::getPoint(2);
}

const MapPoint* MapPoint::getHoliday() {
	return MapHelper::getPoint(3);
}

const MapPoint* MapPoint::getBench() {
	return MapHelper::getPoint(4);
}

const MapPoint* MapPoint::next() const {
	return MapHelper::getPoint(nid);
}

const MapPoint* MapPoint::next_entry() const {
	return MapHelper::getPoint(enid);
}

bool MapPoint::operator==(const int that) const {
	return this->pid == that;
}

bool MapPoint::operator==(const MapPoint& that) const {
	return this->pid == that.pid;
}

MapPoint::MapPoint(const vector<string>& vecStr) :
	entry(vecStr[3] == "0" ? false : true),
	exit(vecStr[4] == "0" ? false : true),
	pid(stoi(vecStr[0])),
	nid(stoi(vecStr[5])),
	enid(stoi(vecStr[6])),
	pointType((PointType)stoi(vecStr[7])),
	title(vecStr[8]),
	note(vecStr[9]),
	eid_start(stoi(vecStr[10])),
	eid_end(stoi(vecStr[11])),
	params(getParams(vecStr[12])) {
}

MapPoint::~MapPoint() {

}
