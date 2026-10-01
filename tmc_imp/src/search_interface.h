#pragma once

#include "./graph.h"

namespace core
{
	class search_interface
	{
	public:
		search_interface(info_reporter *const _ir) : ir(_ir) {};
		virtual void solve(const graph &_g, double _theta)
		{
			this->iters = 0.0;
			this->mr.clear();
			this->sg_bak = _g.data();
#ifdef debug
			this->ivpm = _g.im();
#endif
			this->theta = _theta;
		};
		list result() const
		{
			return this->mr;
		};
#ifdef debug
		void print_mr(list &_mr) const
		{
			if (!_mr.size())
			{
				cout << "none" << endl;
				return;
			}

			list tmr;
			for (const auto &v : _mr)
			{
				tmr.emplace_back((*this->ivpm)[v]);
			}
			stable_sort(tmr.begin(), tmr.end());
			cout << to_string(tmr) << endl;
		};
		void print(const subgraph &_sg, list &_mr) const
		{
			if (!_sg.size() || !_mr.size())
			{
				cout << "none" << endl;
				return;
			}

			sl_pair_list spl;
			for (const auto v : _mr)
			{
				spl.emplace_back(pair{ v, (LL)(*this->ivpm)[v] });
			}
			stable_sort(spl.begin(), spl.end(), sl_pair_cmp);
			
			for (const auto &v : spl)
			{
				const auto &nsv = _sg[v.first];
				cout << v.second << ": ";

				for (ST i = 0; i < 3; ++i)
				{
					for (const auto &u : spl)
					{
						if (u.first != v.first && bs(nsv[i], 0, (ST)nsv[i].size() - 1, u.first) != ST_INVALID)
						{
							cout << (*this->ivpm)[u.first] << " ";
						}
					}

					if (i == 2)
					{
						cout << endl;
					}
					else
					{
						cout << "| ";
					}
				}
			}
		};
#endif
	protected:
		info_reporter *const ir;
		double iters = 0.0;
		list mr;
		const subgraph *sg_bak = nullptr;
#ifdef debug
		const list *ivpm = nullptr;
#endif
		double theta = 0.0;
	};
}
