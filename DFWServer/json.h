#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>

enum struct value_type;

class json {
public:
	json();
	json(bool);
	json(int8_t);
	json(int16_t);
	json(int32_t);
	json(int64_t);
	json(uint8_t);
	json(uint16_t);
	json(uint32_t);
	json(uint64_t);
	json(const std::string&);
	json(std::string&&);
	json(const std::vector<json>&);
	json(std::vector<json>&&);
	json(const std::unordered_map<std::string, json>&);
	json(std::unordered_map<std::string, json>&&);
	json(value_type, void*);
	json(const json&);
	json(json&&);
	json& operator=(const json&);
	json& operator=(json&&);
	~json();

	static json create(const std::string&);
	static json create(const char*);

	std::string serialize() const;

	value_type get_type() const;

	bool get_bool() const;
	int64_t get_int() const;
	std::string get_string() const;
	std::vector<json> get_vector() const;
	std::unordered_map<std::string, json> get_object() const;
	const std::string* convert_string() const;
	const std::vector<json>* convert_vector() const;
	const std::unordered_map<std::string, json>* convert_object() const;
	std::string* convert_string();
	std::vector<json>* convert_vector();
	std::unordered_map<std::string, json>* convert_object();

	// Ver 2.0 - Indexer
	json& operator[](std::size_t index);
	json& operator[](const std::string& index);
	json& operator[](std::string&& index);
	const json& operator[](std::size_t index) const;
	const json& operator[](const std::string& index) const;
	const json& operator[](std::string&& index) const;

	void push_back(const json& that);
	void emplace_back(json&& that);

	void insert(const std::string& key, const json& that);
	void insert(const std::string& key, json&& that);
	void insert(std::string&& key, const json& that);
	void insert(std::string&& key, json&& that);

private:
	value_type json_type;
	void* object;

	void release();

	static value_type read_type(const std::string& input, std::size_t& index);
	static std::string read_string(const std::string& input, std::size_t& index);
	static void* null_parser(const std::string& input, std::size_t& index);
	static bool* bool_parser(const std::string& input, std::size_t& index);
	static int64_t* number_parser(const std::string& input, std::size_t& index);
	static std::string* string_parser(const std::string& input, std::size_t& index);
	static std::vector<json>* array_parser(const std::string& input, std::size_t& index);
	static std::unordered_map<std::string, json>* object_parser(const std::string& input, std::size_t& index);
};

enum struct value_type {
	Null,
	Boolean,
	Number,
	String,
	Array,
	Object,
	Invalid
};
