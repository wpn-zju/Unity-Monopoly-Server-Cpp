#include "json.h"

#include <algorithm>

using namespace std;

json::json() : json_type(value_type::Null), object(nullptr) {

}

json::json(bool value) : json_type(value_type::Boolean), object(new bool(value)) {

}

json::json(int8_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(int16_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(int32_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(int64_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(uint8_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(uint16_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

json::json(uint32_t value) : json_type(value_type::Number), object(new int64_t(value)) {

}

// Truncate
json::json(uint64_t value) : json_type(value_type::Number), object(new int64_t(max(value, static_cast<uint64_t>(INT64_MAX)))) {

}

json::json(const string& value) : json_type(value_type::String), object(new string(value)) {

};

json::json(string&& value) : json_type(value_type::String), object(new string(move(value))) {

}

json::json(const vector<json>& vec) : json_type(value_type::Array), object(new vector<json>(vec)) {

};

json::json(vector<json>&& vec) : json_type(value_type::Array), object(new vector<json>(move(vec))) {

}

json::json(const unordered_map<string, json>& value) : json_type(value_type::Object), object(new unordered_map<string, json>(value)) {

};

json::json(unordered_map<string, json>&& obj) : json_type(value_type::Object), object(new unordered_map<string, json>(move(obj))) {

}

json::json(value_type valueType, void* object) : json_type(valueType), object(object) {

}

json json::create(const string& input) {
	size_t index = 0;

	switch (read_type(input, index))
	{
	case value_type::Boolean:
		return { value_type::Boolean, bool_parser(input, index) };
	case value_type::Number:
		return { value_type::Number, number_parser(input, index) };
	case value_type::String:
		return { value_type::String, string_parser(input, index) };
	case value_type::Array:
		return { value_type::Array, array_parser(input, index) };
	case value_type::Object:
		return { value_type::Object, object_parser(input, index) };
	default:
		return {};
	}

	return {};
}

json json::create(const char* input) { return create(string(input)); }

json::json(const json& that) {
	this->json_type = that.json_type == value_type::Invalid ? value_type::Null : that.json_type;

	switch (this->json_type)
	{
	case value_type::Boolean:
		this->object = new bool(that.get_bool());
		break;
	case value_type::Number:
		this->object = new int64_t(that.get_int());
		break;
	case value_type::String:
		this->object = new string(that.get_string());
		break;
	case value_type::Array:
		this->object = new vector<json>();
		for (auto it = that.convert_vector()->cbegin(); it != that.convert_vector()->cend(); ++it) {
			this->convert_vector()->push_back(*it);
		}
		break;
	case value_type::Object:
		this->object = new unordered_map<string, json>();
		for (auto it = that.convert_object()->cbegin(); it != that.convert_object()->cend(); ++it) {
			this->convert_object()->insert({ it->first, it->second });
		}
		break;
	default:
		this->object = nullptr;
		break;
	}
}

json::json(json&& that) {
	this->json_type = that.json_type;
	this->object = that.object;

	that.json_type = value_type::Null;
	that.object = nullptr;
}

json& json::operator=(const json& that) {
	this->release();

	this->json_type = that.json_type == value_type::Invalid ? value_type::Null : that.json_type;

	switch (this->json_type)
	{
	case value_type::Boolean:
		this->object = new bool(that.get_bool());
		break;
	case value_type::Number:
		this->object = new int64_t(that.get_int());
		break;
	case value_type::String:
		this->object = new string(that.get_string());
		break;
	case value_type::Array:
		this->object = new vector<json>();
		for (auto it = that.convert_vector()->cbegin(); it != that.convert_vector()->cend(); ++it) {
			this->convert_vector()->push_back(*it);
		}
		break;
	case value_type::Object:
		this->object = new unordered_map<string, json>();
		for (auto it = that.convert_object()->cbegin(); it != that.convert_object()->cend(); ++it) {
			this->convert_object()->insert({ it->first, it->second });
		}
		break;
	default:
		this->object = nullptr;
		break;
	}

	return *this;
}

json& json::operator=(json&& that) {
	this->release();

	this->json_type = that.json_type;
	this->object = that.object;

	that.json_type = value_type::Null;
	that.object = nullptr;

	return *this;
}

value_type json::read_type(const string& input, size_t& index) {
	while (input[index] == ' '
		|| input[index] == '\r'
		|| input[index] == '\n'
		|| input[index] == '\t') {
		++index;
		if (index == input.size()) {
			return value_type::Invalid;
		}
	}

	switch (input[index])
	{
	case '"':
		return value_type::String;
	case 't':
		return value_type::Boolean;
	case 'f':
		return value_type::Boolean;
	case '[':
		return value_type::Array;
	case '{':
		return value_type::Object;
	case 'n':
		return value_type::Null;
	case '-':
		return value_type::Number;
	default:
		return input[index] >= '0' && input[index] <= '9' ? value_type::Number : value_type::Invalid;
	}
}

string json::serialize() const {
	string output;

	switch (this->json_type)
	{
	case value_type::Boolean:
		output += this->get_bool() ? "true" : "false";
		break;
	case value_type::Number:
		output += to_string(this->get_int());
		break;
	case value_type::String:
		output += '"' + *this->convert_string() + '"';
		break;
	case value_type::Array: {
		output += '[';
		auto it = this->convert_vector()->cbegin();
		while (it != this->convert_vector()->cend()) {
			output += it->serialize();
			if (++it == this->convert_vector()->cend())
				break;
			output += ',';
		}
		output += ']';
		break;
	}
	case value_type::Object: {
		output += '{';
		auto it = this->convert_object()->cbegin();
		while (it != this->convert_object()->cend()) {
			output += '"' + it->first + '"';
			output += ':';
			output += it->second.serialize();
			if (++it == this->convert_object()->cend())
				break;
			output += ',';
		}
		output += '}';
		break;
	}
	default:
		output += "null";
		break;
	}

	return output;
}

void json::release() {
	switch (this->json_type)
	{
	case value_type::Boolean:
		delete (bool*)(this->object);
		break;
	case value_type::Number:
		delete (int64_t*)(this->object);
		break;
	case value_type::String:
		delete (this->convert_string());
		break;
	case value_type::Array:
		delete (this->convert_vector());
		break;
	case value_type::Object:
		delete (this->convert_object());
		break;
	default:
		break;
	}

	this->json_type = value_type::Null;
	this->object = nullptr;
}

json::~json() {
	this->release();
}

value_type json::get_type() const {
	return this->json_type;
}

bool json::get_bool() const {
	if (this->json_type != value_type::Boolean) {
		throw runtime_error("Invalid calling json::get_bool(), source: " + serialize());
	}

	return *static_cast<bool*>(this->object);
}

int64_t json::get_int() const {
	if (this->json_type != value_type::Number) {
		throw runtime_error("Invalid calling json::get_int(), source: " + serialize());
	}

	return *static_cast<int64_t*>(this->object);
}

// Returning Object - Copy needed
string json::get_string() const {
	if (this->json_type != value_type::String) {
		throw runtime_error("Invalid calling json::get_string(), source: " + serialize());
	}

	return *static_cast<string*>(this->object);
}

// Returning Object - Copy needed
vector<json> json::get_vector() const {
	if (this->json_type != value_type::Array) {
		throw runtime_error("Invalid calling json::get_vector(), source: " + serialize());
	}

	return *static_cast<vector<json>*>(this->object);
}

// Returning Object - Copy needed
unordered_map<string, json> json::get_object() const {
	if (this->json_type != value_type::Object) {
		throw runtime_error("Invalid calling json::get_object(), source: " + serialize());
	}

	return *static_cast<unordered_map<string, json>*>(this->object);
}

// Returning Pointer - Copy free
const string* json::convert_string() const {
	if (this->json_type != value_type::String) {
		throw runtime_error("Invalid calling json::convert_string(), source: " + serialize());
	}

	return static_cast<string*>(this->object);
}

// Returning Pointer - Copy free
const vector<json>* json::convert_vector() const {
	if (this->json_type != value_type::Array) {
		throw runtime_error("Invalid calling json::convert_vector(), source: " + serialize());
	}

	return static_cast<vector<json>*>(this->object);
}

// Returning Pointer - Copy free
const unordered_map<string, json>* json::convert_object() const {
	if (this->json_type != value_type::Object) {
		throw runtime_error("Invalid calling json::convert_object(), source: " + serialize());
	}

	return static_cast<unordered_map<string, json>*>(this->object);
}

// Returning Pointer - Copy free
string* json::convert_string() {
	if (this->json_type != value_type::String) {
		throw runtime_error("Invalid calling json::convert_string(), source: " + serialize());
	}

	return static_cast<string*>(this->object);
}

// Returning Pointer - Copy free
vector<json>* json::convert_vector() {
	if (this->json_type != value_type::Array) {
		throw runtime_error("Invalid calling json::convert_vector(), source: " + serialize());
	}

	return static_cast<vector<json>*>(this->object);
}

// Returning Pointer - Copy free
unordered_map<string, json>* json::convert_object() {
	if (this->json_type != value_type::Object) {
		throw runtime_error("Invalid calling json::convert_object(), source: " + serialize());
	}

	return static_cast<unordered_map<string, json>*>(this->object);
}

// Ver 2.0
json& json::operator[](size_t index) {
	if (this->json_type == value_type::Array) {
		if (index >= 0 && index < this->convert_vector()->size()) {
			return (*this->convert_vector())[index];
		}

		throw runtime_error("Invalid index, want to get: " + to_string(index) + ", actual size: " + to_string(this->convert_vector()->size()));
	}

	throw runtime_error("You can not call integer-indexer for a non-array json, source: " + serialize());
}

json& json::operator[](const string& index) {
	if (this->json_type == value_type::Object) {
		return (*this->convert_object())[index];
	}

	throw runtime_error("You can not call string-indexer for a non-object json, source: " + serialize());
}

json& json::operator[](string&& index) {
	if (this->json_type == value_type::Object) {
		return (*this->convert_object())[index];
	}

	throw runtime_error("You can not call string-indexer for a non-object json, source: " + serialize());
}

const json& json::operator[](size_t index) const {
	if (this->json_type == value_type::Array) {
		if (index >= 0 && index < this->convert_vector()->size()) {
			return (*this->convert_vector())[index];
		}

		throw runtime_error("Invalid index, want to get: " + to_string(index) + ", actual size: " + to_string(this->convert_vector()->size()));
	}

	throw runtime_error("You can not call integer-indexer for a non-array json, source: " + serialize());
}

const json& json::operator[](const string& index) const {
	if (this->json_type == value_type::Object) {
		return (*this->convert_object()).at(index);
	}

	throw runtime_error("You can not call string-indexer for a non-object json, source: " + serialize());
}

const json& json::operator[](string&& index) const {
	if (this->json_type == value_type::Object) {
		return (*this->convert_object()).at(index);
	}

	throw runtime_error("You can not call string-indexer for a non-object json, source: " + serialize());
}

/* Unsafe */
void json::push_back(const json& that) {
	if (this->json_type == value_type::Array) {
		this->convert_vector()->push_back(that);
	}

	throw runtime_error("You can not call json::push_back(const json&) for a non-array json, source: " + serialize());
}

/* Unsafe */
void json::emplace_back(json&& that) {
	if (this->json_type == value_type::Array) {
		this->convert_vector()->emplace_back(that);
	}

	throw runtime_error("You can not call json::emplace_back(json&&) for a non-array json, source: " + serialize());
}

/* Unsafe */
void json::insert(const string& key, const json& that) {
	if (this->json_type == value_type::Object) {
		this->convert_object()->insert(pair<string, json>(key, that));
	}

	throw runtime_error("You can not call json::insert(const std::string& key, const json& that) for a non-object json, source: " + serialize());
}

/* Unsafe */
void json::insert(const string& key, json&& that) {
	if (this->json_type == value_type::Object) {
		this->convert_object()->insert(pair<string, json>(key, move(that)));
	}

	throw runtime_error("You can not call json::insert(const std::string& key, json&& that) for a non-object json, source: " + serialize());
}

/* Unsafe */
void json::insert(string&& key, const json& that) {
	if (this->json_type == value_type::Object) {
		this->convert_object()->insert(pair<string, json>(move(key), that));
	}

	throw runtime_error("You can not call json::insert(std::string&& key, const json& that) for a non-object json, source: " + serialize());
}

/* Unsafe */
void json::insert(string&& key, json&& that) {
	if (this->json_type == value_type::Object) {
		this->convert_object()->insert(std::make_pair(move(key), move(that)));
	}

	throw runtime_error("You can not call json::insert(std::string&& key, json&& that) for a non-object json, source: " + serialize());
}

/* Unsafe */
void* json::null_parser(const string& input, size_t& index) {
	//if (index == input.size() || input[index] != 'n') throw runtime_error("Unexpected Token at pos " + to_string(index) + ", source: " + input);
	//++index;
	//if (index == input.size() || input[index] != 'u') throw runtime_error("Unexpected Token at pos " + to_string(index) + ", source: " + input);
	//++index;
	//if (index == input.size() || input[index] != 'l') throw runtime_error("Unexpected Token at pos " + to_string(index) + ", source: " + input);
	//++index;
	//if (index == input.size() || input[index] != 'l') throw runtime_error("Unexpected Token at pos " + to_string(index) + ", source: " + input);
	//++index;
	//
	//return nullptr

	index = index + 4;

	return nullptr;
}

/* Unsafe */
int64_t* json::number_parser(const string& input, size_t& index) {
	bool neg = input[index] == '-';

	index += neg;

	int64_t* p = new int64_t(0);

	while (input[index] <= '9' && input[index] >= '0') {
		*p = *p * 10;
		*p = *p + input[index] - '0';
		++index;
	}

	if (neg) *p *= -1;

	return p;
}

/* Unsafe */
bool* json::bool_parser(const string& input, size_t& index) {
	//if (input[index] == 't') {
	//	++index;
	//	if (index == input.size() || input[index] != 'u') return nullptr;
	//	++index;
	//	if (index == input.size() || input[index] != 'r') return nullptr;
	//	++index;
	//	if (index == input.size() || input[index] != 'e') return nullptr;
	//	++index;
	//	
	//	return new bool(true);
	//}
	//else if (input[index] == 'f') {
	//	++index;
	//	if (index == input.size() || input[index] != 'a') return nullptr;
	//	++index;
	//	if (index == input.size() || input[index] != 'l') return nullptr;
	//	++index;
	//	if (index == input.size() || input[index] != 's') return nullptr;
	//	++index;
	//	if (index == input.size() || input[index] != 'e') return nullptr;
	//	++index;

	//	return new bool(false);
	//}

	//return nullptr;

	bool t = input[index] == 't';
	index = index + 5 - t;
	return new bool(t);
}

/* Unsafe */
string json::read_string(const string& input, size_t& index) {
	string ret;

	++index;

	// CHECK OVERFLOW
	while (input[index] != '"') {
		// Escape Characters
		if (input[index] == '\\') {
			ret.push_back(input[index]);
			++index;
		}
		ret.push_back(input[index]);
		++index;
	}

	++index;

	return ret;
}

/* Unsafe */
string* json::string_parser(const string& input, size_t& index) {
	return new string(read_string(input, index));
}

vector<json>* json::array_parser(const string& input, size_t& index) {
	vector<json>* p = new vector<json>();

	++index;

	while (input[index] != ']') {
		switch (input[index])
		{
		case ' ':
		case '\r':
		case '\n':
		case '\t':
		case ',':
			++index;
			break;
		case '[':
			p->push_back(json(value_type::Array, array_parser(input, index)));
			break;
		case '{':
			p->push_back(json(value_type::Object, object_parser(input, index)));
			break;
		case 't':
			p->push_back(json(value_type::Boolean, bool_parser(input, index)));
			break;
		case 'f':
			p->push_back(json(value_type::Boolean, bool_parser(input, index)));
			break;
		case '"':
			p->push_back(json(value_type::String, string_parser(input, index)));
			break;
		case 'n':
			p->push_back(json(value_type::Null, null_parser(input, index)));
			break;
		case '-':
			p->push_back(json(value_type::Number, number_parser(input, index)));
			break;
		default:
			p->push_back(json(value_type::Number, number_parser(input, index)));
			break;
		}
	}

	++index;

	return p;
}

/* Unsafe */
unordered_map<string, json>* json::object_parser(const string& input, size_t& index) {
	unordered_map<string, json>* p = new unordered_map<string, json>();

	++index;

	int flag = 0;
	string column = "";
	while (input[index] != '}') {
		if (flag == 1) {
			switch (input[index])
			{
			case ' ':
			case '\r':
			case '\n':
			case '\t':
			case ',':
			case ':':
				++index;
				break;
			case '[':
				p->insert({ column, json(value_type::Array, array_parser(input, index)) });
				flag = 0; column = "";
				break;
			case '{':
				p->insert({ column, json(value_type::Object, object_parser(input, index)) });
				flag = 0; column = "";
				break;
			case 't':
				p->insert({ column, json(value_type::Boolean, bool_parser(input, index)) });
				flag = 0; column = "";
				break;
			case 'f':
				p->insert({ column, json(value_type::Boolean, bool_parser(input, index)) });
				flag = 0; column = "";
				break;
			case '"':
				p->insert({ column, json(value_type::String, string_parser(input, index)) });
				flag = 0; column = "";
				break;
			case 'n':
				p->insert({ column, json(value_type::Null, null_parser(input, index)) });
				flag = 0; column = "";
				break;
			case '-':
				p->insert({ column, json(value_type::Number, number_parser(input, index)) });
				flag = 0; column = "";
				break;
			default:
				p->insert({ column, json(value_type::Number, number_parser(input, index)) });
				flag = 0; column = "";
				break;
			}
		}
		else {
			if (input[index] == '"') {
				column = read_string(input, index);
				flag = 1;
			}
			else {
				++index;
			}
		}
	}

	++index;

	return p;
}
