#include "Utility.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

int clamp(int value, int low, int high) {
	return max(low, min(high, value));
}

vector<string> split(const string& source, char separator) {
	vector<string> vec;

	istringstream f(source);
	string s;
	while (getline(f, s, separator)) {
		vec.push_back(s);
	}

	return vec;
}
