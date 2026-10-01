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
			this->vpm = _g.priority();
			this->theta = _theta;
		};
		list result() const
		{
			return this->mr;
		};
#ifdef debug
		void print_mr(const subgraph &_sg, list &_mr) const
		{
			if (!_sg.size() || !_mr.size())
			{
				cout << "none" << endl;
				return;
			}
			stable_sort(_mr.begin(), _mr.end());

			for (const auto v : _mr)
			{
				const auto &nsv = _sg.find(v)->second;
				cout << v << ": ";

				for (ST i = 0; i < 3; ++i)
				{
					for (const auto u : _mr)
					{
						if (u != v && nsv[i].find(u) != nsv[i].end())
						{
							cout << u << " ";
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
		const list *vpm = nullptr;
		double theta = 0.0;
	};
}
