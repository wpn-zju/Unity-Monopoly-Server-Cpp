#pragma once

#include <vector>
#include <mutex>
#include <stdexcept>
#include "json.h"

class Event {
public:
	operator bool();
	int operator[](size_t) const;

	bool eventSet(std::vector<int>&&);
	bool eventSet(std::vector<json>&&);
	void eventReset();

	Event();
	Event(const Event&) = delete;
	Event(Event&&) = delete;
	Event& operator=(const Event&) = delete;
	Event& operator=(Event&&) = delete;
	~Event();

private:
	bool lock;
	std::mutex mtx;
	std::vector<int> params;
};
