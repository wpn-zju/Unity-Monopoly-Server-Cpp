#pragma once

#include <vector>
#include <string>

int clamp(int, int, int);

std::vector<std::string> split(const std::string&, char);

struct EnumClassHash {
	template <typename T>
	std::size_t operator()(T t) const {
		return static_cast<std::size_t>(t);
	}
};
