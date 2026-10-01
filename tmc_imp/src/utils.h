#pragma once

//#define win
//#define debug
//#define mem

#ifdef win
#include <Windows.h>
#include <Psapi.h>
#else
#include <unistd.h>
#endif

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <chrono>
#include <regex>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::array;
using std::string;
using std::ostringstream;
using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::regex;
using std::sregex_token_iterator;
using std::remove_if;
using std::ifstream;
using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::pair;

namespace core
{
	// ST should be signed types
	using ST = int;
	using LL = long long;
	using ULL = unsigned long long;

	using set = unordered_set<ST>;
	using degrees = array<ST, 3ULL>;
	using sd_pair = pair<ST, degrees>;
	using sd_list = vector<sd_pair>;
	using list = vector<ST>;
	using neighbors = array<list, 3ULL>;
	using subgraph = vector<neighbors>;
	using sl_pair = pair<ST, LL>;
	using sl_pair_list = vector<sl_pair>;

	constexpr ST ST_INVALID = 2147483647;
	constexpr void mask(ST &_v)
	{
		_v = _v | 0x80000000;
	};
	constexpr ST unmask(ST _v)
	{
		return _v & 0x7fffffff;
	};
	constexpr bool masked(ST _v)
	{
		return (_v & 0x80000000) >> 31;
	};

	bool sl_pair_cmp(const sl_pair&, const sl_pair&);
	ST bs(const list&, ST, ST, ST);
	void split(const string&, const string&, vector<string>&);
	void core_decompose(const vector<set>&, list&, list&);
	string to_string(const list&);
#ifdef debug
	bool is_identical(const sd_list&, const sd_list&, ST, ST);
#endif

	class timer
	{
	public:
		void clear();
		ULL elapse(bool=true);
	private:
		system_clock::time_point lastTime, currentTime;
	};

	class info_reporter
	{
	public:
		void clear();
		ULL get_time();
		void print_mem(const string&, bool=false);
	private:
		const ULL span = 500;
#ifdef win
		const double M = 1024 * 1024;
		void get_memory_usage(const string&) const;
#else
		const string data_mem = "VmRSS:";
		void get_memory_usage(int, const string&) const;
#endif
		timer tt, mt;
	};
}
