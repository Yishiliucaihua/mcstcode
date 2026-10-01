#include "./mcst.h"

using namespace core;

void mcst::solve(const graph &_g, double _theta)
{
	search_interface::solve(_g, _theta);

	ST targetK = 0;
	double p = this->theta / (2.0 - this->theta);

	if (_g.method() == graph::sort_method::opt_o)
	{
		const auto &pl = *_g.priority_value();
		ST va = (ST)pl.size(), start, end = va - 1, lastC;

		while (end >= 0)
		{
			lastC = (ST)(pl[end].second / va);
			if (lastC < targetK)
			{
				break;
			}

			for (start = end - 1; start >= 0; --start)
			{
				if ((ST)(pl[start].second / va) != lastC)
				{
					break;
				}
			}
			++start;

			for (ST i = start; i <= end; ++i)
			{
				const auto v = pl[i].first;
				v_info R, C;

				// generate induced subgraph
				this->gen_subgraph(*this->sg_bak, this->sg, v);

				// size pruning
				if (this->sg.size() <= this->mr.size())
				{
					continue;
				}

				// construct search branch
				this->construct(v, R, C);

				if (R.size() + C.size() > this->mr.size())
				{
					this->solve_imp(0, true, R, C);
					targetK = (ST)ceil(this->mr.size() * p);
				}
			}
			end = start - 1;
		}
	}
	else
	{
		st_pair_list cl;
		for (const auto &v : *this->sg_bak)
		{
			cl.emplace_back(st_pair{ v.first, (*this->vpm)[v.first] });
		}
		stable_sort(cl.begin(), cl.end(), st_pair_cmp);

		for (const auto &v : cl)
		{
			const auto vi = v.first;
			v_info R, C;

			this->gen_subgraph(*this->sg_bak, this->sg, vi);

			if (this->sg.size() <= this->mr.size())
			{
				continue;
			}

			this->construct(vi, R, C);

			if (R.size() + C.size() > this->mr.size())
			{
				this->solve_imp(0, true, R, C);
			}
		}
	}
	cout << "number of iterations:=>{" << this->iters << "}" << endl;
}

void mcst::solve_imp(ST _ps, bool _pf, v_info &_R, v_info &_C)
{
#ifdef mem
	this->ir->print_mem("running");
#endif
	++this->iters;

	// advanced pruning
	ST nps = _ps;
	if (this->prt)
	{
		if (_ps < 2)
		{
			ST ocs = (ST)_C.size();
			if (this->adv_prune(_R, _C))
			{
				return;
			}

			if ((ST)_C.size() == ocs)
			{
				++nps;
			}
			else
			{
				nps = 0;
			}
		}
		else
		{
			nps = 0;
		}
	}
	else
	{
		this->prune((ST)_R.size(), _C);
	}

	if (_R.size() + _C.size() <= this->mr.size())
	{
		return;
	}

	// update result
	if (_R.size() > this->mr.size() && this->check(_R))
	{
		this->mr.clear();
		for (const auto &v : _R)
		{
			this->mr.emplace_back(v.first);
		}
	}

	bool npf = false;
	this->tempS.clear();

	if (_pf)
	{
		this->pivot((ST)_R.size(), _C);
		npf = this->tempS.size() ? false : true;
	}
	else
	{
		npf = true;
	}

	// sort vertices
	st_pair_list cl;
	for (const auto &v : _C)
	{
		if (this->tempS.find(v.first) == this->tempS.end())
		{
			cl.emplace_back(st_pair{ v.first, (*this->vpm)[v.first] });
		}
	}
	stable_sort(cl.begin(), cl.end(), st_pair_cmp);

	// enumeration
	ST cur = 0, cls = (ST)cl.size();
	v_info nR, nC;

	while (cur < cls && _R.size() + _C.size() > this->mr.size())
	{
		const auto vi = cl[cur++].first;
		this->updateR(vi, _C, _R, nR);
		this->updateC(vi, _C, nC);
		_C.erase(vi);

		if (nR.size() + nC.size() > this->mr.size())
		{
			this->solve_imp(nps, npf, nR, nC);
		}
	}
}

void mcst::gen_subgraph(const subgraph &_gi, subgraph &_go, ST _v)
{
	auto set_intersection = [](const set &_S, const neighbors &_N, neighbors &_nN) -> void
	{
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : _N[i])
			{
				if (_S.find(u) != _S.end())
				{
					_nN[i].emplace(u);
				}
			}
		}
	};

	_go.clear();
	this->tempS.clear();
	const auto &nsv = _gi.find(_v)->second;

	// collect vertex set
	this->tempS.emplace(_v);
	for (ST i = 0; i < 3; ++i)
	{
		for (const auto u : nsv[i])
		{
			if ((*this->vpm)[u] > (*this->vpm)[_v])
			{
				this->tempS.emplace(u);
			}
		}
	}

	for (const auto u : this->tempS)
	{
		neighbors nsu;
		set_intersection(this->tempS, _gi.find(u)->second, nsu);
		_go.emplace(u, nsu);
	}
}

void mcst::construct(ST _v, v_info &_R, v_info &_C) const
{
	_R.emplace(_v, degrees{ 0, 0, 0 });

	const auto &nsv = this->sg.find(_v)->second;
	for (const auto &u : this->sg)
	{
		if (u.first != _v)
		{
			ST cur = ST_INVALID;
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u.first) != nsv[i].end())
				{
					cur = i;
					break;
				}
			}

			if (cur != ST_INVALID)
			{
				degrees du = { 0, 0, 0 };
				++du[cur];
				_C.emplace(u.first, du);
			}
		}
	}
}

void mcst::updateR(ST _v, const v_info &_C, const v_info &_R, v_info &_nR) const
{
	const auto &nsv = this->sg.find(_v)->second;

	_nR = _R;
	for (auto &u : _nR)
	{
		for (ST i = 0; i < 3; ++i)
		{
			if (nsv[i].find(u.first) != nsv[i].end())
			{
				++u.second[i];
				break;
			}
		}
	}
	_nR.emplace(_v, _C.find(_v)->second);
}

void mcst::updateC(ST _v, const v_info &_C, v_info &_nC) const
{
	const auto &nsv = this->sg.find(_v)->second;

	_nC.clear();
	for (const auto &u : _C)
	{
		if (u.first != _v)
		{
			ST cur = ST_INVALID;
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u.first) != nsv[i].end())
				{
					cur = i;
					break;
				}
			}

			if (cur != ST_INVALID)
			{
				degrees td = u.second;
				++td[cur];
				_nC.emplace(u.first, td);
			}
		}
	}
}

bool mcst::check(const v_info &_R) const
{
	double theta = this->theta;
	for (const auto &v : _R)
	{
		const auto &dv = v.second;
		if ((double)dv[2] / ((max)(dv[0], dv[1]) + dv[2]) < theta)
		{
			return false;
		}
	}
	return true;
}

void mcst::prune(ST _rs, v_info &_C)
{
	this->tempS.clear();
	ST targetC = (ST)this->mr.size() - _rs, tc;

	for (const auto &v : _C)
	{
		const auto vi = v.first;
		const auto &nsv = this->sg.find(vi)->second;
		tc = 0;

		for (const auto &u : _C)
		{
			const auto ui = u.first;
			if (ui != vi && (nsv[0].find(ui) != nsv[0].end() || nsv[1].find(ui) != nsv[1].end() || nsv[2].find(ui) != nsv[2].end()))
			{
				++tc;
				if (tc >= targetC)
				{
					break;
				}
			}
		}

		if (tc < targetC)
		{
			this->tempS.emplace(vi);
		}
	}

	for (const auto v : this->tempS)
	{
		_C.erase(v);
	}
}

ST mcst::intersection_cnt_R(ST _v, const v_info &_S, ST _cst2, ST _cst)
{
	const auto &nsv = this->sg.find(_v)->second;
	this->tempS1.clear();
	this->tempS2.clear();
	ST tc2 = 0, tc = 0;

	for (const auto &u : _S)
	{
		if (nsv[2].find(u.first) != nsv[2].end())
		{
			++tc2;
			++tc;
			this->tempS2.emplace(u.first);

			if (tc > _cst)
			{
				return 1;
			}
		}
	}
	this->tempS1 = this->tempS2;

	if (tc2 >= _cst2)
	{
		for (const auto &u : _S)
		{
			if (nsv[0].find(u.first) != nsv[0].end() || nsv[1].find(u.first) != nsv[1].end())
			{
				++tc;
				this->tempS2.emplace(u.first);

				if (tc > _cst)
				{
					break;
				}
			}
		}
	}

	if (tc2 < _cst2 || tc < _cst)
	{
		return 0;
	}
	return 1;
}

ST mcst::intersection_cnt_C(ST _v, const v_info &_S, ST _cst2, ST _cst)
{
	const auto &nsv = this->sg.find(_v)->second;
	ST tc2 = 0, tc = 0;

	for (const auto &u : _S)
	{
		if (nsv[2].find(u.first) != nsv[2].end())
		{
			++tc2;
			++tc;

			if (tc >= _cst)
			{
				return 1;
			}
		}
	}

	if (tc2 >= _cst2)
	{
		for (const auto &u : _S)
		{
			if (nsv[0].find(u.first) != nsv[0].end() || nsv[1].find(u.first) != nsv[1].end())
			{
				++tc;
				if (tc >= _cst)
				{
					break;
				}
			}
		}
	}

	if (tc2 < _cst2 || tc < _cst)
	{
		return 0;
	}
	return 1;
}

ST mcst::intersection_cnt_P(ST _v, const v_info &_S)
{
	const auto &nsv = this->sg.find(_v)->second;
	this->tempS1.clear();
	ST tc2 = 0;

	for (const auto &u : _S)
	{
		if (nsv[2].find(u.first) != nsv[2].end())
		{
			++tc2;
			this->tempS1.emplace(u.first);
		}
	}
	return tc2;
}

void mcst::pivot(ST _rs, const v_info &_C)
{
	this->tempSPL.clear();
	ST maxD = ST_INVALID, tc = (ST)_C.size() - 1, td;
	double maxSum = 0.0;

	for (const auto &v : _C)
	{
		if (v.second[2] == _rs)
		{
			this->tempSPL.emplace_back(sl_pair{ v.first, (LL)(*this->vpm)[v.first] });
		}
	}
	stable_sort(this->tempSPL.begin(), this->tempSPL.end(), sl_pair_cmp);

	for (const auto &v : this->tempSPL)
	{
		td = this->intersection_cnt_P(v.first, _C);
		if (td == tc)
		{
			this->tempS.swap(this->tempS1);
			return;
		}

		if (td > maxD)
		{
			maxD = td;
			maxSum = 0.0;

			for (const auto u : this->tempS1)
			{
				maxSum += (*this->vpm)[u];
			}
			this->tempS.swap(this->tempS1);
		}
		else if (td == maxD)
		{
			double sum = 0.0;
			for (const auto u : this->tempS1)
			{
				sum += (*this->vpm)[u];
			}

			if (sum > maxSum)
			{
				maxSum = sum;
				this->tempS.swap(this->tempS1);
			}
		}
	}
}

bool mcst::adv_prune(v_info &_R, v_info &_C)
{
	double p = this->theta / (1.0 - this->theta), pt = this->theta / (2.0 - this->theta);
	ST targetC = (ST)(this->mr.size() + 1 - _R.size()), temp, cur, ttc;
	list tl;

	for (const auto &v : _R)
	{
		temp = (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2];
		tl.emplace_back(temp);

		if (temp > targetC)
		{
			targetC = temp;
		}
	}

	if ((ST)_C.size() < targetC)
	{
		return true;
	}

	cur = 0;
	temp = (ST)ceil((_R.size() + targetC - 1) * pt);

	for (const auto &v : _R)
	{
		tl[cur] = (max)(tl[cur], temp - v.second[2]);
		++cur;
	}

	// process R
	this->tempS.clear();
	cur = 0;

	for (const auto &v : _R)
	{
		ttc = (max)(targetC, tl[cur]);
		if (ttc > 0 && !this->intersection_cnt_R(v.first, _C, tl[cur], ttc))
		{
			return true;
		}

		if ((ST)this->tempS2.size() == ttc)
		{
			this->tempS.insert(this->tempS2.begin(), this->tempS2.end());
		}
		if ((ST)this->tempS1.size() == tl[cur])
		{
			this->tempS.insert(this->tempS1.begin(), this->tempS1.end());
		}
		++cur;
	}

	// process critical vertices
	if (this->tempS.size())
	{
		if (!this->verify(_R, _C))
		{
			return true;
		}
		else
		{
			targetC = (ST)(this->mr.size() + 1 - _R.size());
			for (const auto &v : _R)
			{
				temp = (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2];
				if (temp > targetC)
				{
					targetC = temp;
				}
			}

			if ((ST)_C.size() < targetC)
			{
				return true;
			}
		}
	}

	// process C
	this->tempS.clear();
	temp = (ST)ceil((_R.size() + targetC - 1) * pt);
	--targetC;

	for (const auto &v : _C)
	{
		ST valve = (max)(temp - v.second[2], (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2]);
		ttc = (max)(targetC, valve);

		if (ttc > 0 && !this->intersection_cnt_C(v.first, _C, valve, ttc))
		{
			this->tempS.emplace(v.first);
		}
	}

	for (const auto v : this->tempS)
	{
		_C.erase(v);
	}
	return false;
}

bool mcst::verify(v_info &_R, v_info &_C)
{
	auto is_contained = [this]() -> bool
	{
		if (this->tempS.size() > this->tempC.size())
		{
			return false;
		}

		for (const auto u : this->tempS)
		{
			if (this->tempC.find(u) == this->tempC.end())
			{
				return false;
			}
		}
		return true;
	};

	const auto vi = *this->tempS.begin();
	this->tempS.erase(vi);
	this->updateR(vi, _C, _R, this->tempR);
	this->updateC(vi, _C, this->tempC);

	if (!is_contained())
	{
		return false;
	}
	_R.swap(this->tempR);
	_C.swap(this->tempC);

	while (this->tempS.size())
	{
		const auto vi = *this->tempS.begin();
		this->tempS.erase(vi);
		this->updateR(vi, _C, _R, this->tempR);
		this->updateC(vi, _C, this->tempC);

		if (!is_contained())
		{
			return false;
		}
		_R.swap(this->tempR);
		_C.swap(this->tempC);
	}
	return true;
}
