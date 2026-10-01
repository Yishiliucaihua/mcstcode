#include "./utils.h"

using namespace core;

bool core::sl_pair_cmp(const sl_pair &_p1, const sl_pair &_p2)
{
	return _p1.second < _p2.second;
}

ST core::bs(const list &_arr, ST _s, ST _e, ST _key)
{
	ST mid;
	while (_s <= _e)
	{
		mid = (_s + _e) >> 1;
		if (_arr[mid] == _key)
		{
			return mid;
		}
		else if (_arr[mid] > _key)
		{
			_e = mid - 1;
		}
		else
		{
			_s = mid + 1;
		}
	}
	return ST_INVALID;
}

void core::split(const string &_str, const string &_del, vector<string> &_con)
{
	_con.clear();
	if (_str.size())
	{
		regex reg("\\" + _del);
		sregex_token_iterator iter(_str.begin(), _str.end(), reg, -1), end;

		while (iter != end)
		{
			_con.emplace_back(iter->str());
			++iter;
		}
	}
}

void core::core_decompose(const vector<set> &_g, list &_deg, list &_order)
{
	ST n = (ST)_g.size();
	list vert(n), pos(n), bin;

	// init deg
	ST md = 0;
	for (ST i = 0; i < n; ++i)
	{
		_deg[i] = (ST)_g[i].size();
		if (_deg[i] > md)
		{
			md = _deg[i];
		}
	}

	// insert to bins
	bin.resize((ULL)md + 1, 0);
	for (ST i = 0; i < n; ++i)
	{
		++bin[_deg[i]];
	}

	// set start pos of each bin
	ST start = 0, num;
	for (ST i = 0; i <= md; ++i)
	{
		num = bin[i];
		bin[i] = start;
		start += num;
	}

	// associate bin, pos and vert
	for (ST i = 0; i < n; ++i)
	{
		pos[i] = bin[_deg[i]]++;
		vert[pos[i]] = i;
	}

	// recover bin
	for (ST i = md; i >= 1; --i)
	{
		bin[i] = bin[(ULL)i - 1];
	}
	bin[0] = 0;

	// peeling
	ST v, w, du, pu, pw;
	for (ST i = 0; i < n; ++i)
	{
		v = vert[i];
		_order[v] = i;

		for (const auto u : _g[v])
		{
			if (_deg[u] > _deg[v])
			{
				du = _deg[u];
				pu = pos[u];
				pw = bin[du];
				w = vert[pw];

				if (u != w)
				{
					pos[u] = pw;
					vert[pu] = w;
					pos[w] = pu;
					vert[pw] = u;
				}

				++bin[du];
				--_deg[u];
			}
		}
	}
}

static ostringstream oss;
string core::to_string(const list &_l)
{
	if (!_l.size())
	{
		return string("none");
	}

	oss.str("");
	ST n = (ST)_l.size();

	oss << _l[0];
	for (ST i = 1; i < n; ++i)
	{
		oss << "_" << _l[i];
	}
	return oss.str();
}

#ifdef debug
bool core::is_identical(const sd_list &_sdl1, const sd_list &_sdl2, ST _s, ST _e)
{
	for (ST i = _s; i <= _e; ++i)
	{
		const auto &item1 = _sdl1[i], &item2 = _sdl2[i];
		if (item1.first != item2.first)
		{
			return false;
		}

		for (ST j = 0; j < 3; ++j)
		{
			if (item1.second[j] != item2.second[j])
			{
				return false;
			}
		}
	}
	return true;
}
#endif

void timer::clear()
{
	this->currentTime = this->lastTime = system_clock::now();
}

ULL timer::elapse(bool _clear)
{
	this->currentTime = system_clock::now();
	ULL ret = duration_cast<milliseconds>(this->currentTime - this->lastTime).count();
	if (_clear)
	{
		this->lastTime = this->currentTime;
	}
	return ret;
}

void info_reporter::clear()
{
	this->tt.clear();
	this->mt.clear();
}

ULL info_reporter::get_time()
{
	return this->tt.elapse();
}

void info_reporter::print_mem(const string &_prefix, bool _force)
{
	if (_force || this->mt.elapse(false) > this->span)
	{
#ifdef win
		this->get_memory_usage(_prefix);
#else
		this->get_memory_usage(getpid(), _prefix);
#endif
		this->mt.clear();
	}
}

#ifdef win
void info_reporter::get_memory_usage(const string &_prefix) const
{
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		cout << _prefix << " mem:=>{" << fixed << setprecision(2) << pmc.WorkingSetSize / M << "}" << endl;
	}
}
#else
void info_reporter::get_memory_usage(int _pid, const string &_prefix) const
{
	oss.str("");
	oss.clear();
	oss << "/proc/" << _pid << "/status";

	ifstream ifs(oss.str());
	if (ifs.is_open())
	{
		string line;
		ST ps = (ST)this->data_mem.size();

		while (getline(ifs, line))
		{
			if (!line.compare(0, ps, this->data_mem, 0, ps))
			{
				line = line.substr(ps, line.size() - ps - 2);
				line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
				cout << _prefix << " mem:=>{" << fixed << setprecision(2) << atoi(line.c_str()) / 1024.0 << "}" << endl;
				break;
			}
		}
	}
}
#endif
