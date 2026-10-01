#include "./mcst.h"

using namespace core;

void mcst::solve(const graph &_g, double _theta)
{
	search_interface::solve(_g, _theta);

	ST targetK = 0;
	double p = this->theta / (2.0 - this->theta);
	const auto &pl = *_g.priority_value();

	if (_g.method() == graph::sort_method::opt_o)
	{
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
				// generate induced subgraph
				const auto vi = pl[i].first;
				this->gen_subgraph(*this->sg_bak, this->sg, vi);

				// size pruning
				if (this->sg.size() <= this->mr.size())
				{
					continue;
				}

				// construct subgraph & search branch
				this->construct(this->mapping[vi]);

				// 1+|C|
				if (this->root_pos + 2 > (ST)this->mr.size())
				{
					this->solve_imp(0, true, this->root_pos);
					targetK = (ST)ceil(this->mr.size() * p);
				}
			}
			end = start - 1;
		}
	}
	else
	{
		for (const auto &v : pl)
		{
			const auto vi = v.first;
			this->gen_subgraph(*this->sg_bak, this->sg, vi);

			if (this->sg.size() <= this->mr.size())
			{
				continue;
			}

			this->construct(this->mapping[vi]);

			if (this->root_pos + 2 > (ST)this->mr.size())
			{
				this->solve_imp(0, true, this->root_pos);
			}
		}
	}
	cout << "number of iterations:=>{" << this->iters << "}" << endl;
}

void mcst::gen_subgraph(const subgraph &_gi, subgraph &_go, ST _v)
{
	auto set_intersection = [this](const set &_S, const neighbors &_N, neighbors &_nN) -> void
	{
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : _N[i])
			{
				if (_S.find(u) != _S.end())
				{
					_nN[i].emplace_back(this->mapping[u]);
				}
			}
		}
	};

	_go.clear();
	this->tempS.clear();
	const auto &nsv = _gi[_v];

	// collect vertex set
	this->tempS.emplace(_v);
	for (ST i = 0; i < 3; ++i)
	{
		for (const auto u : nsv[i])
		{
			if (u > _v)
			{
				this->tempS.emplace(u);
			}
		}
	}

	this->tempL.clear();
	this->im.clear();
	this->mapping.clear();

	for (const auto v : this->tempS)
	{
		this->tempL.emplace_back(v);
	}
	stable_sort(this->tempL.begin(), this->tempL.end());

	ST va = (ST)this->tempL.size();
	this->im.resize(va, va);
	this->tempSD.reserve(va);

	for (ST i = 0; i < va; ++i)
	{
		this->mapping.emplace(this->tempL[i], i);
		this->im[i] = this->tempL[i];
	}

	this->sg.clear();
	this->sg.resize(va);
	this->v2i.resize(va, va);

	for (const auto v : this->tempL)
	{
		auto &nv = this->sg[this->mapping[v]];
		set_intersection(this->tempS, _gi[v], nv);
	}
}

void mcst::construct(ST _v)
{
	ST va = (ST)this->sg.size();

	// search branch
	this->R.clear();
	this->R.emplace(_v, degrees{ 0, 0, 0 });
	this->C.clear();
	this->C.resize(va);

	const auto &nsv = this->sg[_v];
	for (ST i = 0; i < 3; ++i)
	{
		for (const auto u : nsv[i])
		{
			degrees ds = { 0, 0, 0 };
			++ds[i];
			this->C[u] = pair{ u, ds };
		}
	}

	ST cur = 0;
	for (ST i = 0; i < va; ++i)
	{
		if (this->C[i].second[0] || this->C[i].second[1] || this->C[i].second[2])
		{
			if (i != cur)
			{
				this->C[cur].swap(this->C[i]);
			}
			++cur;
		}
	}
	this->root_pos = cur - 1;
}

void mcst::solve_imp(ST _ps, bool _pf, ST _ce)
{
#ifdef mem
	this->ir->print_mem("running");
#endif
	++this->iters;

	ST ce_prune = _ce, ce_enum;
	ST rs, cs, pv = ST_INVALID;
	ST new_pos;

	// advanced pruning
	ST nps = _ps;
	if (this->prt)
	{
		if (_ps < 2)
		{
			if (this->adv_prune(_ce, ce_prune))
			{
				this->restoreS(ce_prune, _ce);
				return;
			}

			if (_ce == ce_prune)
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
		this->prune(_ce);
		this->collect(_ce);
		this->generateNC(_ce, ce_prune);
	}

	// size
	rs = (ST)this->R.size();
	cs = ce_prune + 1;
	if (rs + cs <= (ST)this->mr.size())
	{
		this->restoreS(ce_prune, _ce);
		return;
	}

	// update result
	if (rs > (ST)this->mr.size() && this->check())
	{
		this->mr.clear();
		for (const auto &v : this->R)
		{
			this->mr.emplace_back(this->im[v.first]);
		}
	}

	// select pivot
	bool npf = false;
	if (_pf)
	{
		pv = this->pivot(rs, ce_prune);
		npf = pv != ST_INVALID ? false : true;
	}
	else
	{
		npf = true;
	}

	// enumeration
	ST cur = 0, pn = 0;
	while (cur <= ce_prune && rs + pn + ce_prune - cur + 1 > (ST)this->mr.size())
	{
		const auto vi = this->C[cur].first;
		if (pv != ST_INVALID)
		{
			const auto &nspv2 = this->sg[pv][2];
			if (bs(nspv2, 0, (ST)nspv2.size() - 1, vi) != ST_INVALID)
			{
				++cur;
				++pn;
				continue;
			}
		}

		this->updateR(vi, cur++);
		this->markC(vi, pv, ce_prune);
		this->collect(ce_prune);
		this->generateNC(ce_prune, ce_enum);

		if (this->R.size() + ce_enum + 1 > this->mr.size())
		{
			new_pos = ce_enum;
			this->solve_imp(nps, npf, new_pos);
		}

		// restore C & R
		this->restoreNC(vi, ce_enum);
		this->restoreC(ce_enum, ce_prune);
		this->restoreR(vi);
	}
	this->restoreS(ce_prune, _ce);
}

bool mcst::check() const
{
	if (this->R.size() == 1)
	{
		return true;
	}

	for (const auto &v : this->R)
	{
		const auto &dv = v.second;
		if ((double)dv[2] / ((max)(dv[0], dv[1]) + dv[2]) < this->theta)
		{
			return false;
		}
	}
	return true;
}

void mcst::updateR(ST _v, ST _cur)
{
	const auto &nsv = this->sg[_v];
	this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 += 1.0;
		}
	}
	cost2 *= this->R.size();

	if (cost2 < cost1)
	{
		for (auto &v : this->R)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (bs(nsv[i], 0, this->ts[i], v.first) != ST_INVALID)
				{
					++v.second[i];
					break;
				}
			}
		}
	}
	else
	{
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				auto iter = this->R.find(u);
				if (iter != this->R.end())
				{
					++iter->second[i];
				}
			}
		}
	}
	this->R.emplace(this->C[_cur]);
}

void mcst::restoreR(ST _v)
{
	this->R.erase(_v);
	const auto &nsv = this->sg[_v];
	this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 += 1.0;
		}
	}
	cost2 *= this->R.size();

	if (cost2 < cost1)
	{
		for (const auto &v : this->R)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (bs(nsv[i], 0, this->ts[i], v.first) != ST_INVALID)
				{
					--this->R.find(v.first)->second[i];
					break;
				}
			}
		}
	}
	else
	{
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				auto iter = this->R.find(u);
				if (iter != this->R.end())
				{
					--iter->second[i];
				}
			}
		}
	}
}

bool mcst::cost_cmp(const neighbors &_nsv, ST _cs)
{
	this->ts = { (ST)_nsv[0].size() - 1, (ST)_nsv[1].size() - 1, (ST)_nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 += 1.0;
		}
	}

	cost1 = cost1 * 2.0 + _cs * 2.0;
	cost2 *= _cs;

	if (cost1 < cost2)
	{
		return true;
	}
	return false;
}

void mcst::markC(ST _v, ST _pv, ST _ce)
{
	ST _cb = 0;
	if (_cb <= _ce)
	{
		const auto &nsv = this->sg[_v];
		if (this->cost_cmp(nsv, _ce))
		{
			if (_pv != ST_INVALID)
			{
				const auto &nspv2 = this->sg[_pv][2];
				ST b1 = 0, e1 = (ST)nspv2.size() - 1, v1, v2;

				while (b1 <= e1 && _cb <= _ce)
				{
					v1 = nspv2[b1];
					v2 = this->C[_cb].first;

					if (v2 > _v)
					{
						break;
					}

					if (v1 < v2)
					{
						++b1;
					}
					else if (v1 == v2)
					{
						++b1;
						++_cb;
					}
					else
					{
						mask(this->C[_cb++].first);
					}
				}
			}

			while (_cb <= _ce && this->C[_cb].first <= _v)
			{
				mask(this->C[_cb++].first);
			}

			degrees nsb = { 0, 0, 0 }, nse{};
			loser_tree lt(3);
			ST minV, minI, cmpV;
			_cb = 0;

			for (ST i = 0; i < 3; ++i)
			{
				nse[i] = this->ts[i];
				if (nsb[i] > nse[i])
				{
					lt.update_item(i, ST_INVALID);
				}
				else
				{
					lt.update_item(i, nsv[i][0]);
				}
			}

			while ((minV = lt.top(minI)) != ST_INVALID)
			{
				while (_cb <= _ce)
				{
					cmpV = this->C[_cb].first;
					if (masked(cmpV))
					{
						++_cb;
						continue;
					}

					if (cmpV < minV)
					{
						mask(this->C[_cb++].first);
					}
					else if (cmpV == minV)
					{
						++this->C[_cb++].second[minI];
						break;
					}
					else
					{
						break;
					}
				}

				if (_cb > _ce)
				{
					return;
				}

				++nsb[minI];
				if (nsb[minI] > nse[minI])
				{
					lt.update_item(minI, ST_INVALID);
				}
				else
				{
					lt.update_item(minI, nsv[minI][nsb[minI]]);
				}
			}

			while (_cb <= _ce)
			{
				mask(this->C[_cb++].first);
			}
		}
		else
		{
			if (_pv != ST_INVALID)
			{
				const auto &nspv2 = this->sg[_pv][2];
				while (_cb <= _ce)
				{
					const auto u = this->C[_cb].first;
					if (u > _v)
					{
						break;
					}

					if (bs(nspv2, 0, (ST)nspv2.size() - 1, u) == ST_INVALID)
					{
						mask(this->C[_cb].first);
					}
					++_cb;
				}
			}

			while (_cb <= _ce && this->C[_cb].first <= _v)
			{
				mask(this->C[_cb++].first);
			}

			_cb = 0;
			for (ST i = _cb; i <= _ce; ++i)
			{
				const auto u = this->C[i].first;
				if (masked(u))
				{
					continue;
				}

				bool flag = false;
				for (ST j = 0; j < 3; ++j)
				{
					if (bs(nsv[j], 0, this->ts[j], u) != ST_INVALID)
					{
						++this->C[i].second[j];
						flag = true;
						break;
					}
				}

				if (!flag)
				{
					mask(this->C[i].first);
				}
			}
		}
	}
}

void mcst::collect(ST _ce)
{
	this->tempSD.clear();
	for (ST i = 0; i <= _ce; ++i)
	{
		const auto &v = this->C[i];
		if (masked(v.first))
		{
			this->tempSD.emplace_back(sd_pair{ unmask(v.first), v.second });
		}
	}
}

void mcst::generateNC(ST _ce, ST &_new_ce)
{
	ST _cb = 0;
	for (ST i = _cb; i <= _ce; ++i)
	{
		if (!masked(this->C[i].first))
		{
			if (_cb != i)
			{
				this->C[_cb] = this->C[i];
			}
			++_cb;
		}
	}

	copy(this->tempSD.begin(), this->tempSD.end(), this->C.begin() + _cb);
	_new_ce = _cb - 1;
}

void mcst::restoreNC(ST _v, ST _new_ce)
{
	ST _cb = 0;
	if (_cb <= _new_ce)
	{
		const auto &nsv = this->sg[_v];
		if (this->cost_cmp(nsv, _new_ce))
		{
			degrees nsb = { 0, 0, 0 }, nse{};
			loser_tree lt(3);
			ST minV, minI, cmpV;

			for (ST i = 0; i < 3; ++i)
			{
				nse[i] = this->ts[i];
				if (nsb[i] > nse[i])
				{
					lt.update_item(i, ST_INVALID);
				}
				else
				{
					lt.update_item(i, nsv[i][0]);
				}
			}

			while ((minV = lt.top(minI)) != ST_INVALID)
			{
				while (_cb <= _new_ce)
				{
					cmpV = this->C[_cb].first;
					if (cmpV < minV)
					{
						++_cb;
					}
					else if (cmpV == minV)
					{
						--this->C[_cb++].second[minI];
						break;
					}
					else
					{
						break;
					}
				}

				if (_cb > _new_ce)
				{
					return;
				}

				++nsb[minI];
				if (nsb[minI] > nse[minI])
				{
					lt.update_item(minI, ST_INVALID);
				}
				else
				{
					lt.update_item(minI, nsv[minI][nsb[minI]]);
				}
			}
		}
		else
		{
			for (ST i = _cb; i <= _new_ce; ++i)
			{
				auto &u = this->C[i];
				for (ST j = 0; j < 3; ++j)
				{
					if (bs(nsv[j], 0, this->ts[j], u.first) != ST_INVALID)
					{
						--u.second[j];
						break;
					}
				}
			}
		}
	}
}

void mcst::restoreC(ST _ce_enum, ST _ce)
{
	this->tempSD.clear();
	ST _cb = 0, new_cb = _ce_enum + 1;
	if (_cb >= new_cb || _ce_enum >= _ce)
	{
		return;
	}

	ST v1, v2;
	while (_cb <= _ce_enum && new_cb <= _ce)
	{
		v1 = this->C[_cb].first;
		v2 = this->C[new_cb].first;

		if (v1 > v2)
		{
			this->tempSD.emplace_back(this->C[new_cb++]);
		}
		else
		{
			this->tempSD.emplace_back(this->C[_cb++]);
		}
	}

	while (_cb <= _ce_enum)
	{
		this->tempSD.emplace_back(this->C[_cb++]);
	}
	while (new_cb <= _ce)
	{
		this->tempSD.emplace_back(this->C[new_cb++]);
	}
	copy(this->tempSD.begin(), this->tempSD.end(), this->C.begin());
}

void mcst::restoreS(ST _ce_prune, ST _ce)
{
	if (_ce_prune >= _ce)
	{
		return;
	}

	this->tempL.clear();
	for (ST i = _ce_prune + 1; i <= _ce; ++i)
	{
		auto &v = this->C[i];
		if (masked(v.first))
		{
			v.first = unmask(v.first);
			this->tempL.emplace_back(v.first);
		}
	}

	// restore needed values in C
	for (const auto v : this->tempL)
	{
		this->restoreNC(v, _ce_prune);
	}

	// reorder C
	this->tempSD.clear();
	ST b1 = 0, e1 = _ce_prune, b2 = _ce_prune + 1, e2 = _ce, v1, v2;

	if (b1 < b2)
	{
		while (b1 <= e1 && b2 <= e2)
		{
			v1 = this->C[b1].first;
			v2 = this->C[b2].first;

			if (v1 < v2)
			{
				this->tempSD.emplace_back(this->C[b1++]);
			}
			else
			{
				this->tempSD.emplace_back(this->C[b2++]);
			}
		}

		while (b1 <= e1)
		{
			this->tempSD.emplace_back(this->C[b1++]);
		}
		while (b2 <= e2)
		{
			this->tempSD.emplace_back(this->C[b2++]);
		}
		copy(this->tempSD.begin(), this->tempSD.end(), this->C.begin());
	}

	// recover R
	for (const auto v : this->tempL)
	{
		this->R.erase(v);
		const auto &nsv = this->sg[v];
		this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
		double cost1 = 0.0, cost2 = 0.0;

		for (ST i = 0; i < 3; ++i)
		{
			cost1 += this->ts[i];
			if (this->ts[i] > 1)
			{
				cost2 += log2(this->ts[i]);
			}
			else
			{
				cost2 += 1.0;
			}
		}
		cost2 *= this->R.size();

		if (cost2 < cost1)
		{
			for (const auto &v : this->R)
			{
				for (ST i = 0; i < 3; ++i)
				{
					if (bs(nsv[i], 0, this->ts[i], v.first) != ST_INVALID)
					{
						--this->R.find(v.first)->second[i];
						break;
					}
				}
			}
		}
		else
		{
			for (ST i = 0; i < 3; ++i)
			{
				for (const auto u : nsv[i])
				{
					auto iter = this->R.find(u);
					if (iter != this->R.end())
					{
						--iter->second[i];
					}
				}
			}
		}
	}
}

ST mcst::intersection_cnt_pivot(ST _v, ST _ce, double &_sum)
{
	const auto &nsv2 = this->sg[_v][2];
	ST _cb = 0, tc = 0, ts2 = (ST)nsv2.size() - 1;
	double cost1 = ts2 + _ce, cost2 = 0.0;
	_sum = 0.0;

	if (ts2 > 1)
	{
		cost2 += log2(ts2);
	}
	else
	{
		cost2 += 1.0;
	}
	cost2 *= _ce;

	if (cost2 < cost1)
	{
		for (ST i = _cb; i <= _ce; ++i)
		{
			if (bs(nsv2, 0, ts2, this->C[i].first) != ST_INVALID)
			{
				++tc;
				_sum += this->im[this->C[i].first];
			}
		}
	}
	else
	{
		ST b1 = 0, e1 = ts2, v1, vc;
		while (b1 <= e1 && _cb <= _ce)
		{
			v1 = nsv2[b1];
			vc = this->C[_cb].first;

			if (v1 < vc)
			{
				++b1;
			}
			else if (v1 == vc)
			{
				++tc;
				++b1;
				++_cb;
				_sum += this->im[vc];
			}
			else
			{
				++_cb;
			}
		}
	}
	return tc;
}

ST mcst::pivot(ST _rs, ST _ce_prune)
{
	ST maxV = ST_INVALID, maxD = -1, tc = _ce_prune, td;
	double maxSum = 0.0, sum;

	for (ST i = 0; i <= _ce_prune; ++i)
	{
		const auto &v = this->C[i];
		if (v.second[2] == _rs)
		{
			td = this->intersection_cnt_pivot(v.first, _ce_prune, sum);
			if (td == tc)
			{
				return v.first;
			}

			if (td > maxD)
			{
				maxD = td;
				maxSum = sum;
				maxV = v.first;
			}
			else if (td == maxD)
			{
				if (sum > maxSum)
				{
					maxSum = sum;
					maxV = v.first;
				}
			}
		}
	}
	return maxV;
}

ST mcst::intersection_cnt_prune(ST _v, ST _ce, ST _target)
{
	const auto &nsv = this->sg[_v];
	ST _cb = 0, tc = 0;

	if (this->cost_cmp(nsv, _ce))
	{
		degrees nsb = { 0, 0, 0 }, nse{};
		loser_tree lt(3);
		ST minV, minI, cmpV;

		for (ST i = 0; i < 3; ++i)
		{
			nse[i] = this->ts[i];
			if (nsb[i] > nse[i])
			{
				lt.update_item(i, ST_INVALID);
			}
			else
			{
				lt.update_item(i, nsv[i][0]);
			}
		}

		while ((minV = lt.top(minI)) != ST_INVALID)
		{
			while (_cb <= _ce)
			{
				cmpV = this->C[_cb].first;
				if (cmpV < minV)
				{
					++_cb;
				}
				else if (cmpV == minV)
				{
					++tc;
					++_cb;
					if (tc >= _target)
					{
						return tc;
					}
					break;
				}
				else
				{
					break;
				}
			}

			if (_cb > _ce)
			{
				break;
			}

			++nsb[minI];
			if (nsb[minI] > nse[minI])
			{
				lt.update_item(minI, ST_INVALID);
			}
			else
			{
				lt.update_item(minI, nsv[minI][nsb[minI]]);
			}
		}
	}
	else
	{
		for (ST i = _cb; i <= _ce; ++i)
		{
			for (ST j = 0; j < 3; ++j)
			{
				if (bs(nsv[j], 0, this->ts[j], this->C[i].first) != ST_INVALID)
				{
					++tc;
					break;
				}
			}

			if (tc >= _target)
			{
				return tc;
			}
		}
	}
	return tc;
}

void mcst::prune(ST _ce)
{
	this->tempL.clear();
	ST target = (ST)(this->mr.size() - this->R.size());

	for (ST i = 0; i <= _ce; ++i)
	{
		if (this->intersection_cnt_prune(this->C[i].first, _ce, target) < target)
		{
			this->tempL.emplace_back(i);
		}
	}

	for (const auto i : this->tempL)
	{
		mask(this->C[i].first);
	}
}

bool mcst::intersection_adv_prune_R(const sd_list &_C, ST _v, ST _ce, ST _target2, ST _target)
{
	this->tempS1.clear();
	this->tempS2.clear();
	const auto &nsv = this->sg[_v];
	ST tc2 = 0, tc = 0;
	this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 += 1;
		}
	}

	cost1 += _ce * 2.0;
	cost2 *= _ce;

	if (cost2 < cost1)
	{
		for (ST i = 0; i <= _ce; ++i)
		{
			const auto u = _C[i].first;
			if (bs(nsv[2], 0, this->ts[2], u) != ST_INVALID)
			{
				this->tempS2.emplace(u);
				++tc2;

				if (tc2 > _target)
				{
					return true;
				}
			}
		}

		tc = tc2;
		if (tc2 >= _target2)
		{
			for (ST i = 0; i <= _ce; ++i)
			{
				const auto u = _C[i].first;
				for (ST j = 0; j < 2; ++j)
				{
					if (bs(nsv[j], 0, this->ts[j], u) != ST_INVALID)
					{
						++tc;
						this->tempS1.emplace(u);
						break;
					}
				}

				if (tc > _target)
				{
					break;
				}
			}
		}
	}
	else
	{
		ST b2 = 0, e2 = this->ts[2], b3 = 0, e3 = _ce, v2, v3;
		while (b2 <= e2 && b3 <= e3)
		{
			v2 = nsv[2][b2];
			v3 = _C[b3].first;

			if (v2 < v3)
			{
				++b2;
			}
			else if (v2 == v3)
			{
				++tc2;
				this->tempS2.emplace(v3);

				if (tc2 > _target)
				{
					return true;
				}

				++b2;
				++b3;
			}
			else
			{
				++b3;
			}
		}

		tc = tc2;
		if (tc2 >= _target2)
		{
			degrees nsb = { 0, 0 }, nse{};
			loser_tree lt(2);
			ST minV, minI;

			for (ST i = 0; i < 2; ++i)
			{
				nse[i] = this->ts[i];
				if (nsb[i] > nse[i])
				{
					lt.update_item(i, ST_INVALID);
				}
				else
				{
					lt.update_item(i, nsv[i][0]);
				}
			}

			b3 = 0, e3 = _ce;
			while ((minV = lt.top(minI)) != ST_INVALID)
			{
				while (b3 <= e3)
				{
					v3 = _C[b3].first;
					if (v3 < minV)
					{
						++b3;
					}
					else if (v3 == minV)
					{
						++tc;
						++b3;
						this->tempS1.emplace(v3);
						break;
					}
					else
					{
						break;
					}
				}

				if (b3 > e3 || tc > _target)
				{
					break;
				}

				if (++nsb[minI] > nse[minI])
				{
					lt.update_item(minI, ST_INVALID);
				}
				else
				{
					lt.update_item(minI, nsv[minI][nsb[minI]]);
				}
			}
		}
	}

	if (tc2 < _target2 || tc < _target)
	{
		return false;
	}
	return true;
}

bool mcst::intersection_adv_prune_C(const sd_list &_C, ST _v, ST _ce, ST _target2, ST _target)
{
	const auto &nsv = this->sg[_v];
	ST tc2 = 0, tc = 0;
	this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 += 1.0;
		}
	}

	cost1 += _ce * 2.0;
	cost2 *= _ce;

	if (cost2 < cost1)
	{
		for (ST i = 0; i <= _ce; ++i)
		{
			if (bs(nsv[2], 0, this->ts[2], _C[i].first) != ST_INVALID)
			{
				++tc2;
				if (tc2 >= _target)
				{
					return true;
				}
			}
		}

		tc = tc2;
		if (tc2 >= _target2)
		{
			for (ST i = 0; i <= _ce; ++i)
			{
				const auto u = _C[i].first;
				for (ST j = 0; j < 2; ++j)
				{
					if (bs(nsv[j], 0, this->ts[j], u) != ST_INVALID)
					{
						++tc;
						break;
					}
				}

				if (tc >= _target)
				{
					break;
				}
			}
		}
	}
	else
	{
		ST b2 = 0, e2 = this->ts[2], b3 = 0, e3 = _ce, v2, v3;
		while (b2 <= e2 && b3 <= e3)
		{
			v2 = nsv[2][b2];
			v3 = _C[b3].first;

			if (v2 < v3)
			{
				++b2;
			}
			else if (v2 == v3)
			{
				++tc2;
				if (tc2 >= _target)
				{
					return true;
				}

				++b2;
				++b3;
			}
			else
			{
				++b3;
			}
		}

		tc = tc2;
		if (tc2 >= _target2)
		{
			degrees nsb = { 0, 0 }, nse{};
			loser_tree lt(2);
			ST minV, minI;

			for (ST i = 0; i < 2; ++i)
			{
				nse[i] = this->ts[i];
				if (nsb[i] > nse[i])
				{
					lt.update_item(i, ST_INVALID);
				}
				else
				{
					lt.update_item(i, nsv[i][0]);
				}
			}

			b3 = 0, e3 = _ce;
			while ((minV = lt.top(minI)) != ST_INVALID)
			{
				while (b3 <= e3)
				{
					v3 = _C[b3].first;
					if (v3 < minV)
					{
						++b3;
					}
					else if (v3 == minV)
					{
						++tc;
						++b3;
						break;
					}
					else
					{
						break;
					}
				}

				if (b3 > e3 || tc >= _target)
				{
					break;
				}

				if (++nsb[minI] > nse[minI])
				{
					lt.update_item(minI, ST_INVALID);
				}
				else
				{
					lt.update_item(minI, nsv[minI][nsb[minI]]);
				}
			}
		}
	}

	if (tc2 < _target2 || tc < _target)
	{
		return false;
	}
	return true;
}

bool mcst::adv_prune(ST _ce, ST &_ce_prune)
{
	double p = this->theta / (1.0 - this->theta), pt = this->theta / (2.0 - this->theta);
	ST targetC = (ST)(this->mr.size() + 1 - this->R.size()), temp, cur, ttc;
	list tl;

	for (const auto &v : this->R)
	{
		temp = (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2];
		tl.emplace_back(temp);

		if (temp > targetC)
		{
			targetC = temp;
		}
	}

	if (_ce + 1 < targetC)
	{
		return true;
	}

	cur = 0;
	temp = (ST)ceil((this->R.size() + targetC - 1) * pt);

	for (const auto &v : this->R)
	{
		tl[cur] = (max)(tl[cur], temp - v.second[2]);
		++cur;
	}

	// process R
	this->tempS.clear();
	this->tempL.clear();
	cur = 0;

	for (const auto &v : this->R)
	{
		ttc = (max)(targetC, tl[cur]);
		if (ttc > 0 && !this->intersection_adv_prune_R(this->C, v.first, _ce, tl[cur], ttc))
		{
			return true;
		}

		if ((ST)(this->tempS1.size() + this->tempS2.size()) == ttc)
		{
			this->tempS.insert(this->tempS1.begin(), this->tempS1.end());
			this->tempS.insert(this->tempS2.begin(), this->tempS2.end());
		}
		else if ((ST)this->tempS2.size() == tl[cur])
		{
			this->tempS.insert(this->tempS2.begin(), this->tempS2.end());
		}
		++cur;
	}

	for (ST i = 0; i <= _ce; ++i)
	{
		this->v2i[this->C[i].first] = i;
	}

	if (this->tempS.size())
	{
		// process critical vertices
		this->tempR = this->R;
		if (!this->verify(_ce))
		{
			this->R.swap(this->tempR);
			return true;
		}
		else
		{
			targetC = (ST)(this->mr.size() + 1 - this->R.size());
			for (const auto &v : this->R)
			{
				temp = (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2];
				if (temp > targetC)
				{
					targetC = temp;
				}
			}

			if ((ST)this->tempS1.size() < targetC)
			{
				this->R.swap(this->tempR);
				return true;
			}

			// generate newC
			this->tempSD.clear();
			for (ST i = 0; i <= _ce; ++i)
			{
				if (this->tempS1.find(this->C[i].first) != this->tempS1.end())
				{
					this->tempSD.emplace_back(*this->deltaC.find(this->C[i].first));
				}
				else
				{
					mask(this->C[i].first);
				}
			}

			// process newC and update C
			ST end = (ST)this->tempSD.size() - 1;
			temp = (ST)ceil((this->R.size() + targetC - 1) * pt);
			--targetC;

			for (const auto &v : this->tempSD)
			{
				ST valve = (max)(temp - v.second[2], (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2]);
				ttc = (max)(targetC, valve);

				if (ttc > 0 && !this->intersection_adv_prune_C(this->tempSD, v.first, end, valve, ttc))
				{
					this->tempL.emplace_back(v.first);
				}
				else
				{
					this->C[this->v2i[v.first]].second = v.second;
				}
			}

			for (const auto v : this->tempL)
			{
				mask(this->C[this->v2i[v]].first);
			}

			this->collect(_ce);
			this->generateNC(_ce, _ce_prune);

			// mask critical vertices
			for (ST i = _ce_prune + 1; i <= _ce; ++i)
			{
				if (this->tempR.find(this->C[i].first) == this->tempR.end() && this->R.find(this->C[i].first) != this->R.end())
				{
					mask(this->C[i].first);
				}
			}
		}
	}
	else
	{
		temp = (ST)ceil((this->R.size() + targetC - 1) * pt);
		--targetC;

		for (ST i = 0; i <= _ce; ++i)
		{
			const auto &v = this->C[i];
			ST valve = (max)(temp - v.second[2], (ST)ceil(p * (max)(v.second[0], v.second[1])) - v.second[2]);
			ttc = (max)(targetC, valve);

			if (ttc > 0 && !this->intersection_adv_prune_C(this->C, v.first, _ce, valve, ttc))
			{
				this->tempL.emplace_back(i);
			}
		}

		for (const auto idx : this->tempL)
		{
			mask(this->C[idx].first);
		}

		this->collect(_ce);
		this->generateNC(_ce, _ce_prune);
	}
	return false;
}

bool mcst::intersection_verify(ST _v)
{
	const auto &nsv = this->sg[_v];
	this->ts = { (ST)nsv[0].size() - 1, (ST)nsv[1].size() - 1, (ST)nsv[2].size() - 1 };
	double cost1 = 0.0, cost2 = 0.0;

	for (ST i = 0; i < 3; ++i)
	{
		cost1 += this->ts[i];
		if (this->ts[i] > 1)
		{
			cost2 += log2(this->ts[i]);
		}
		else
		{
			cost2 = 1.0;
		}
	}

	cost1 *= 2.0;
	cost2 *= (this->tempS1.size() + this->R.size());

	if (cost2 < cost1)
	{
		for (const auto &u : this->R)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (bs(nsv[i], 0, this->ts[i], u.first) != ST_INVALID)
				{
					++this->R.find(u.first)->second[i];
					break;
				}
			}
		}

		auto iter = tempS1.begin();
		while (iter != tempS1.end())
		{
			bool f = false;
			for (ST i = 0; i < 3; ++i)
			{
				if (bs(nsv[i], 0, this->ts[i], *iter) != ST_INVALID)
				{
					++this->deltaC.find(*iter)->second[i];
					++iter;
					f = true;
					break;
				}
			}
			if (!f)
			{
				iter = tempS1.erase(iter);
			}
		}
	}
	else
	{
		this->tempS2.clear();
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				if (this->tempS1.find(u) != this->tempS1.end())
				{
					this->tempS2.emplace(u);
					++this->deltaC.find(u)->second[i];
				}

				auto iter = this->R.find(u);
				if (iter != this->R.end())
				{
					++iter->second[i];
				}
			}
		}
		this->tempS1.swap(this->tempS2);
	}

	if (this->tempS.size() > this->tempS1.size())
	{
		return false;
	}

	for (const auto u : this->tempS)
	{
		if (this->tempS1.find(u) == this->tempS1.end())
		{
			return false;
		}
	}
	return true;
}

bool mcst::verify(ST _ce)
{
	this->tempS1.clear();
	this->deltaC.clear();

	for (ST i = 0; i <= _ce; ++i)
	{
		const auto &v = this->C[i];
		this->tempS1.emplace(v.first);
		this->deltaC.emplace(v);
	}

	// obtain common neighbors of critical vertices
	while (this->tempS.size())
	{
		const auto iter = this->tempS.begin();
		const auto v = *iter;
		this->tempS.erase(iter);

		if (!this->intersection_verify(v))
		{
			return false;
		}

		// insert a vertex into R
		this->R.emplace(v, this->deltaC.find(v)->second);
	}
	return true;
}
