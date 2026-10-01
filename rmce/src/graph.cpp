#include "./graph.h"

using namespace core;

void graph::load(const string &_file_path)
{
	this->va = 0;
	this->ea = { 0, 0 };
	this->dm = { 0, 0 };
	this->sg.clear();
	this->vpm.clear();
	this->pmr.clear();
	this->fl.clear();

	ifstream ifs(_file_path);
	if (ifs.is_open())
	{
		string line;
		vector<string> temp1, temp2;
		ST vn = 0, n, td;

		// load graph to sg
		while (getline(ifs, line))
		{
			if (!this->pmr.size())
			{
				this->pmr.emplace_back(vn);
			}

			neighbors ns;
			split(line, "|", temp1);

			n = (ST)temp1.size();
			for (ST i = 0; i < n; ++i)
			{
				split(temp1[i], ",", temp2);
				for (const auto &v : temp2)
				{
					ns[i].emplace(atoi(v.c_str()));
				}

				if (i < 2)
				{
					this->ea[i] += (ST)temp2.size();
				}
				else
				{
					this->ea[0] += (ST)temp2.size();
					this->ea[1] += (ST)temp2.size();

					if (this->pmr.size() < 2)
					{
						pmr.clear();
						pmr.emplace_back(vn);
						pmr.emplace_back(*ns[i].begin());
					}
				}
			}

			td = (ST)(ns[0].size() + ns[2].size());
			if (td > this->dm[0])
			{
				this->dm[0] = td;
			}
			td = (ST)(ns[1].size() + ns[2].size());
			if (td > this->dm[1])
			{
				this->dm[1] = td;
			}

			this->sg.emplace(vn++, ns);
		}

		this->va = (ST)this->sg.size();
		this->ea[0] >>= 1;
		this->ea[1] >>= 1;
		ifs.close();
	}
}

void graph::print_info() const
{
	cout << "#vertices:" << this->va << endl;
	cout << "#edges1:" << this->ea[0] << endl;
	cout << "#edges2:" << this->ea[1] << endl;
	cout << "average degree1:" << 2.0 * this->ea[0] / this->va << endl;
	cout << "average degree2:" << 2.0 * this->ea[1] / this->va << endl;
	cout << "max degree1:" << this->dm[0] << endl;
	cout << "max degree2:" << this->dm[1] << endl;
}

void graph::reduction_vertex(double _theta)
{
	auto intersection_uw = [](ST _v, const neighbors &_nsu, const neighbors &_nsw) -> bool
	{
		set nsu, nsw;
		for (ST i = 0; i < 3; ++i)
		{
			nsu.insert(_nsu[i].begin(), _nsu[i].end());
			nsw.insert(_nsw[i].begin(), _nsw[i].end());
		}

		if (nsu.size() < nsw.size())
		{
			for (const auto v : nsu)
			{
				if (v != _v && nsw.find(v) != nsw.end())
				{
					return true;
				}
			}
		}
		else
		{
			for (const auto v : nsw)
			{
				if (v != _v && nsu.find(v) != nsu.end())
				{
					return true;
				}
			}
		}
		return false;
	};

	auto is_mr3 = [this, _theta](ST _v, ST _u, ST _w) -> bool
	{
		const auto &nsv2 = this->sg.find(_v)->second[2];
		const auto &nsu2 = this->sg.find(_u)->second[2];
		ST cnt = 0;
		
		if (nsv2.find(_u) != nsv2.end())
		{
			++cnt;
		}
		if (nsv2.find(_w) != nsv2.end())
		{
			++cnt;
		}
		if (nsu2.find(_w) != nsu2.end())
		{
			++cnt;
		}

		if (_theta == 0.5)
		{
			return cnt >= 2 ? true : false;
		}
		else
		{
			return cnt == 3 ? true : false;
		}
	};
	
	set Q;
	for (const auto &v : this->sg)
	{
		const auto &nsv = v.second;
		if (nsv[0].size() + nsv[1].size() + nsv[2].size() <= 2)
		{
			Q.emplace(v.first);
		}
	}

	while (Q.size())
	{
		auto v = *Q.begin();
		Q.erase(v);
		auto &nsv = this->sg.find(v)->second;
		unordered_map<ST, ST> nsvc;
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				nsvc.emplace(u, i);
			}
		}
		ST deg = (ST)nsvc.size();

		if (deg == 1)
		{
			auto iter = nsvc.begin();
			auto u = iter->first, idx = iter->second;
			auto &nsu = this->sg.find(u)->second;
			nsu[idx].erase(v);
			nsv[idx].clear();

			if (nsu[0].size() + nsu[1].size() + nsu[2].size() <= 2 && Q.find(u) == Q.end())
			{
				Q.emplace(u);
			}
		}
		else if (deg == 2)
		{
			ST uw[2] = {}, idx[2] = {}, cur = 0;
			for (const auto p : nsvc)
			{
				uw[cur] = p.first;
				idx[cur] = p.second;
				++cur;
			}

			auto &nsu = this->sg.find(uw[0])->second;
			auto &nsw = this->sg.find(uw[1])->second;

			ST ti = ST_INVALID;
			for (ST i = 0; i < 3; ++i)
			{
				if (nsu[i].find(uw[1]) != nsu[i].end())
				{
					ti = i;
					break;
				}
			}

			if (ti == ST_INVALID)
			{
				nsu[idx[0]].erase(v);
				nsw[idx[1]].erase(v);
			}
			else if (intersection_uw(v, nsu, nsw))
			{
				if (this->pmr.size() < 3 && is_mr3(v, uw[0], uw[1]))
				{
					this->pmr = {v, uw[0], uw[1] };
				}
				nsu[idx[0]].erase(v);
				nsw[idx[1]].erase(v);
			}
			else
			{
				if (this->pmr.size() < 3 && is_mr3(v, uw[0], uw[1]))
				{
					this->pmr = { v, uw[0], uw[1] };
				}
				nsu[idx[0]].erase(v);
				nsw[idx[1]].erase(v);
				nsu[ti].erase(uw[1]);
				nsw[ti].erase(uw[0]);
			}
			nsv[idx[0]].clear();
			nsv[idx[1]].clear();

			if (nsu[0].size() + nsu[1].size() + nsu[2].size() <= 2 && Q.find(uw[0]) == Q.end())
			{
				Q.emplace(uw[0]);
			}
			if (nsw[0].size() + nsw[1].size() + nsw[2]	.size() <= 2 && Q.find(uw[1]) == Q.end())
			{
				Q.emplace(uw[1]);
			}
		}
	}
}

void graph::reduction_edge()
{
	auto intersection_uv = [](const neighbors &_nsu, const neighbors &_nsv) -> ST
	{
		set nsu, nsv;
		for (ST i = 0; i < 3; ++i)
		{
			nsu.insert(_nsu[i].begin(), _nsu[i].end());
			nsv.insert(_nsv[i].begin(), _nsv[i].end());
		}

		if (nsu.size() < nsv.size())
		{
			for (const auto w : nsu)
			{
				if (nsv.find(w) != nsv.end())
				{
					return w;
				}
			}
		}
		else
		{
			for (const auto w : nsv)
			{
				if (nsu.find(w) != nsu.end())
				{
					return w;
				}
			}
		}
		return ST_INVALID;
	};

	vector<set> visited(this->va);
	for (ST u = 0; u < this->va; ++u)
	{
		auto &nsu = this->sg.find(u)->second;
		for (ST i = 0; i < 3; ++i)
		{
			auto iter = nsu[i].begin();
			while (iter != nsu[i].end())
			{
				auto v = *iter;
				if (v > u && visited[u].find(v) == visited[u].end())
				{
					auto &nsv = this->sg.find(v)->second;
					ST w = intersection_uv(nsu, nsv);

					if (w == ST_INVALID)
					{
						iter = nsu[i].erase(iter);
						nsv[i].erase(u);
						continue;
					}
					else
					{
						if (w > u)
						{
							visited[u].emplace(w);
							if (v > w)
							{
								visited[w].emplace(v);
							}
							else
							{
								visited[v].emplace(w);
							}
						}
					}
				}
				++iter;
			}
		}
	}
}

void graph::reorder(sort_method _sm)
{
	ST va = this->va;
	st_pair_list pl(va);
	this->fl.resize(va);
	this->vpm.resize(va);
	
	// sort
	if (_sm == sort_method::degree_o)
	{
		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg.find(i)->second;
			pl[i] = { i, (ST)(v[0].size() + v[1].size() + v[2].size()) };
			this->fl[i] = { i, pl[i].second > 0 };
		}
	}
	else if (_sm == sort_method::core_o)
	{
		vector<set> tg(va);
		list cn(va), order(va);

		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg.find(i)->second;
			tg[i].insert(v[0].begin(), v[0].end());
			tg[i].insert(v[1].begin(), v[1].end());
			tg[i].insert(v[2].begin(), v[2].end());
		}

		core_decompose(tg, cn, order);

		for (ST i = 0; i < va; ++i)
		{
			pl[i] = { i, order[i] };
			this->fl[i] = { i, cn[i] > 0 };
		}
	}
	else
	{
		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg.find(i)->second;
			pl[i] = { i, i };
			this->fl[i] = { i, (ST)(v[0].size() + v[1].size() + v[2].size()) > 0 };
		}
	}

	stable_sort(pl.begin(), pl.end(), st_pair_cmp);
	for (ST i = 0; i < va; ++i)
	{
		this->vpm[pl[i].first] = i;
	}
}
