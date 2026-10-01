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
	using ST = int;
	using LL = long long;
	using ULL = unsigned long long;

	using list = vector<ST>;
	using set = unordered_set<ST>;
	using neighbors = array<set, 3ULL>;
	using subgraph = unordered_map<ST, neighbors>;
	using degrees = array<ST, 3ULL>;
	using v_info = unordered_map<ST, degrees>;
	using st_pair = pair<ST, ST>;
	using st_pair_list = vector<st_pair>;
	using sl_pair = pair<ST, LL>;
	using sl_pair_list = vector<sl_pair>;

	constexpr ST ST_INVALID = -2147483647 - 1;

	bool st_pair_cmp(const st_pair&, const st_pair&);
	bool sl_pair_cmp(const sl_pair&, const sl_pair&);
	void split(const string&, const string&, vector<string>&);
	void core_decompose(const vector<set>&, list&, list&);
	string to_string(const list&);
#ifdef debug
	bool is_identical(const subgraph&, const subgraph&);
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
