#include "./graph.h"

using namespace core;

void graph::load(const string &_file_path)
{
	this->va = 0;
	this->ea = { 0, 0 };
	this->dm = { 0, 0 };
	this->sm = sort_method::none;
	this->sg.clear();
	this->pl.clear();
#ifdef debug
	this->ivpm.clear();
#endif

	ifstream ifs(_file_path);
	if (ifs.is_open())
	{
		string line;
		vector<string> temp1, temp2;
		ST n, td;

		// load graph to sg
		while (getline(ifs, line))
		{
			neighbors ns;
			split(line, "|", temp1);

			n = (ST)temp1.size();
			for (ST i = 0; i < n; ++i)
			{
				split(temp1[i], ",", temp2);
				for (const auto &v : temp2)
				{
					ns[i].emplace_back(atoi(v.c_str()));
				}

				if (i < 2)
				{
					this->ea[i] += (ST)temp2.size();
				}
				else
				{
					this->ea[0] += (ST)temp2.size();
					this->ea[1] += (ST)temp2.size();
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

			this->sg.emplace_back(ns);
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

void graph::reorder(sort_method _sm)
{
	this->sm = _sm;
	ST va = this->va;
	this->pl.resize(va);
	list vpm(va);
#ifdef debug
	this->ivpm.resize(va);
#endif

	// sort
	if (_sm == sort_method::degree_o)
	{
		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg[i];
			this->pl[i] = { i, (LL)(v[0].size() + v[1].size() + v[2].size()) };
		}
	}
	else if (_sm == sort_method::core_o)
	{
		vector<set> tg(va);
		list cn(va), order(va);

		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg[i];
			tg[i].insert(v[0].begin(), v[0].end());
			tg[i].insert(v[1].begin(), v[1].end());
			tg[i].insert(v[2].begin(), v[2].end());
		}
		core_decompose(tg, cn, order);

		for (ST i = 0; i < va; ++i)
		{
			this->pl[i] = { i, (LL)order[i] };
		}
	}
	else if (_sm == sort_method::opt_o)
	{
		vector<set> tg(va);
		list cn(va), order(va);

		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg[i];
			tg[i].insert(v[2].begin(), v[2].end());
		}
		core_decompose(tg, cn, order);

		for (ST i = 0; i < va; ++i)
		{
			const auto &v = this->sg[i];
			this->pl[i] = { i, (LL)va * cn[i] + v[0].size() + v[1].size() + v[2].size() };
		}
	}
	else
	{
		for (ST i = 0; i < va; ++i)
		{
			this->pl[i] = { i, (LL)i };
		}
	}

	// reorder
	stable_sort(this->pl.begin(), this->pl.end(), sl_pair_cmp);
	for (ST i = 0; i < va; ++i)
	{
		vpm[this->pl[i].first] = i;
#ifdef debug
		this->ivpm[i] = this->pl[i].first;
#endif
	}

	subgraph nsg(va);
	for (ST i = 0; i < va; ++i)
	{
		auto &nv = nsg[vpm[i]];
		for (ST j = 0; j < 3; ++j)
		{
			for (const auto u : this->sg[i][j])
			{
				nv[j].emplace_back(vpm[u]);
			}
			stable_sort(nv[j].begin(), nv[j].end());
		}
	}
	this->sg.swap(nsg);

	for (ST i = 0; i < va; ++i)
	{
		this->pl[i].first = i;
	}
}
