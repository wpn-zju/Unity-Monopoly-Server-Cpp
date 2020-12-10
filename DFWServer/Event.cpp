#include "Event.h"

using namespace std;

Event::operator bool() {
	return lock;
}

int Event::operator[](size_t index) const {
	if (index < 0 || index >= params.size()) {
		throw runtime_error("Event indexer overflow.");
	}
	else {
		return params[index];
	}
}

bool Event::eventSet(vector<int>&& parameters) {
	if (lock) {
		return false;
	}
	else {
		unique_lock<mutex> lk(mtx);
		lock = true;
		params = move(parameters);
		return true;
	}
}

bool Event::eventSet(vector<json>&& parameters) {
	if (lock) {
		return false;
	}
	else {
		unique_lock<mutex> lk(mtx);
		lock = true;
		for (const json& value : parameters) params.push_back((int)value.get_int());
		return true;
	}
}

void Event::eventReset() {
	unique_lock<mutex> lk(mtx);
	lock = false;
	params.clear();
}

Event::Event() : lock(false), mtx(), params() {

}

Event::~Event() {

}
