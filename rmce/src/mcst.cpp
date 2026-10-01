#include "./mcst.h"

using namespace core;

void mcst::solve(const graph &_g, double _theta)
{
	search_interface::solve(_g, _theta);

	ST vi, va = (ST)this->sg_bak->size();
	set R, C, X, nX;
	st_pair_list cl;
	this->ignore.resize(va, va);

	const auto &fl = *_g.flag();
	for (const auto &p : fl)
	{
		if (p.second > 0)
		{
			cl.emplace_back(st_pair{ p.first, (*this->vpm)[p.first] });
		}
	}
	stable_sort(cl.begin(), cl.end(), st_pair_cmp);

	for (const auto &v : cl)
	{
		vi = v.first;
		const auto &nsv = this->sg_bak->find(vi)->second;

		// collect candidates
		C.clear();
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				if ((*this->vpm)[u] > (*this->vpm)[vi])
				{
					C.emplace(u);
				}
			}
		}

		// size pruning
		if (C.size() < this->mr.size())
		{
			continue;
		}

		R.clear();
		R.emplace(vi);

		X.clear();
		for (ST i = 0; i < 3; ++i)
		{
			for (const auto u : nsv[i])
			{
				if ((*this->vpm)[u] < (*this->vpm)[vi])
				{
					X.emplace(u);
				}
			}
		}

		// process forbidden set
		this->pruneX(vi, X, C, nX);

		if (R.size() + C.size() > this->mr.size())
		{
			this->solve_imp(R, C, nX);
		}
	}

	cout << "number of iterations:=>{" << this->iters << "}" << endl;
}

void mcst::solve_imp(const set &_R, const set &_C, const set &_X)
{
#ifdef mem
	this->ir->print_mem("running 1");
#endif
	++this->iters;

	if (!_C.size() && !_X.size())
	{
		this->create_sub(_R);
	}
	if (!_C.size())
	{
		return;
	}

	set R = _R, C = _C, X = _X;
	this->dynamic_prune(R, C, X);

	// select pivot
	this->pivot(C, X);

	// collect vertices
	list cl;
	for (const auto v : C)
	{
		if (this->tempS1.find(v) == this->tempS1.end())
		{
			cl.emplace_back(v);
		}
	}

	// enumeration
	ST cur = 0, cls = (ST)cl.size();
	set nC, nX;

	while (cur < cls && R.size() + C.size() > this->mr.size())
	{
		const auto vi = cl[cur++];
		R.emplace(vi);
		C.erase(vi);
		this->updateCX(vi, C, nC);
		this->updateCX(vi, X, nX);

		if (R.size() + nC.size() > this->mr.size())
		{
			this->solve_imp(R, nC, nX);
		}

		R.erase(vi);
		X.emplace(vi);
	}
}

void mcst::dynamic_prune(set &_R, set &_C, set &_X)
{
	// mark vertices
	this->tempS1.clear();
	for (const auto v : _X)
	{
		const auto &nsv = this->sg_bak->find(v)->second;
		for (const auto u : _C)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u) != nsv[i].end())
				{
					this->tempS1.emplace(u);
					break;
				}
			}
		}
	}

	// degree 0 & 1
	this->tempM.clear();
	this->tempS3.clear();
	for (const auto v : _C)
	{
		this->tempS2.clear();
		const auto &nsv = this->sg_bak->find(v)->second;
		for (const auto u : _C)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u) != nsv[i].end())
				{
					this->tempS2.emplace(u);
					break;
				}
			}

			if (this->tempS2.size() > 1)
			{
				break;
			}
		}

		if (!this->tempS2.size())
		{
			if (this->tempS1.find(v) == this->tempS1.end())
			{
				_R.emplace(v);
				this->create_sub(_R);
				_R.erase(v);
			}
			this->tempS3.emplace(v);
		}
		else if (this->tempS2.size() == 1)
		{
			auto u = *this->tempS2.begin();
			if (this->tempS1.find(v) == this->tempS1.end() || this->tempS1.find(u) == this->tempS1.end())
			{
				this->tempM.emplace(v, u);
			}
		}
	}

	auto iter = this->tempM.begin();
	while (iter != this->tempM.end())
	{
		auto v = iter->first, u = iter->second;
		_R.emplace(v);
		_R.emplace(u);
		this->create_sub(_R);
		_R.erase(v);
		_R.erase(u);

		if (this->tempM.find(u) != this->tempM.end())
		{
			this->tempM.erase(u);
			this->tempS3.emplace(u);
		}
		this->tempS3.emplace(v);
		++iter;
	}

	for (const auto v : this->tempS3)
	{
		_C.erase(v);
		_X.emplace(v);
	}
	
	// degree |C|-1
	ST ts = (ST)_C.size() - 1;
	this->tempS2.clear();
	for (const auto v : _C)
	{
		ST dv = 0;
		const auto &nsv = this->sg_bak->find(v)->second;
		for (const auto u : _C)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u) != nsv[i].end())
				{
					++dv;
					break;
				}
			}
		}

		if (dv == ts)
		{
			this->tempS2.emplace(v);
		}
	}

	for (const auto v : this->tempS2)
	{
		_R.emplace(v);
		_C.erase(v);
	}

	// update X
	for (const auto v : this->tempS2)
	{
		this->tempS3.clear();
		const auto &nsv = this->sg_bak->find(v)->second;
		for (const auto u : _X)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u) != nsv[i].end())
				{
					this->tempS3.emplace(u);
					break;
				}
			}
		}
		_X.swap(this->tempS3);
	}

	if (this->tempS2.size() && _C.size() == 0 && _X.size() == 0)
	{
		this->create_sub(_R);
	}
}

bool mcst::contain(ST _u, const set &_nsv_larger, ST _met) const
{
	const auto &nsu = this->sg_bak->find(_u)->second;
	set nsu_larger;
	const auto uo = (*this->vpm)[_u];

	for (ST i = 0; i < 3; ++i)
	{
		for (const auto w : nsu[i])
		{
			if ((*this->vpm)[w] > uo)
			{
				nsu_larger.emplace(w);
			}
		}
	}

	if (_met == 1)
	{
		for (const auto w : _nsv_larger)
		{
			if (w != _u && nsu_larger.find(w) == nsu_larger.end())
			{
				return false;
			}
		}
		return true;
	}
	else if (_met == 2)
	{
		for (const auto w : nsu_larger)
		{
			if (_nsv_larger.find(w) == _nsv_larger.end())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void mcst::pruneX(ST _v, const set &_X, const set &_C, set &_nX)
{
	_nX.clear();
	const auto vo = (*this->vpm)[_v];

	for (const auto u : _X)
	{
		if (this->ignore[u] >= vo)
		{
			_nX.emplace(u);
		}
	}

	for (const auto u : _C)
	{
		if (this->contain(u, _C, 1))
		{
			this->ignore[_v] = (min)((*this->vpm)[u], this->ignore[_v]);
		}
		else if (this->contain(u, _C, 2))
		{
			this->ignore[u] = (min)(vo, this->ignore[u]);
		}
	}
}

ST mcst::intersection_pivot(ST _v, const set &_C)
{
	this->tempS2.clear();
	const auto &nsv = this->sg_bak->find(_v)->second;
	ST tc = 0;

	for (const auto u : _C)
	{
		for (ST i = 0; i < 3; ++i)
		{
			if (nsv[i].find(u) != nsv[i].end())
			{
				++tc;
				this->tempS2.emplace(u);
				break;
			}
		}
	}
	return tc;
}

void mcst::pivot(const set &_C, const set &_X)
{
	this->tempS1.clear();
	ST maxD = -1, td;

	for (const auto v : _C)
	{
		td = this->intersection_pivot(v, _C);
		if (td > maxD)
		{
			this->tempS2.swap(this->tempS1);
			maxD = td;
		}
	}

	for (const auto v : _X)
	{
		td = this->intersection_pivot(v, _C);
		if (td > maxD)
		{
			this->tempS2.swap(this->tempS1);
			maxD = td;
		}
	}
}

void mcst::create_sub(const set &_R)
{
	set F;
	v_info R;

	for (const auto v : _R)
	{
		degrees td = { 0, 0, 0 };
		const auto &nsv = this->sg_bak->find(v)->second;

		for (const auto u : _R)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u) != nsv[i].end())
				{
					++td[i];
					break;
				}
			}
		}
		R.emplace(v, td);
	}
	this->solve_sub(R, F);
}

void mcst::solve_sub(const v_info &_R, const set &_F)
{
#ifdef mem
	this->ir->print_mem("running 2");
#endif

	if (_R.size() <= this->mr.size())
	{
		return;
	}

	if (this->check(_R))
	{
		this->mr.clear();
		for (const auto &v : _R)
		{
			this->mr.emplace_back(v.first);
		}
		return;
	}

	v_info nR;
	list Q;
	set nF = _F;

	for (const auto &v : _R)
	{
		if (nF.find(v.first) == nF.end())
		{
			Q.emplace_back(v.first);
		}
	}

	while (Q.size())
	{
		const auto v = Q.back();
		Q.pop_back();

		nR = _R;
		nR.erase(v);

		const auto &nsv = this->sg_bak->find(v)->second;
		for (auto &u : nR)
		{
			for (ST i = 0; i < 3; ++i)
			{
				if (nsv[i].find(u.first) != nsv[i].end())
				{
					--u.second[i];
					break;
				}
			}
		}

		this->solve_sub(nR, nF);
		nF.emplace(v);
		++this->iters;
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

void mcst::updateCX(ST _v, const set &_C, set &_nC) const
{
	_nC.clear();
	const auto &nsv = this->sg_bak->find(_v)->second;

	for (const auto u : _C)
	{
		for (ST i = 0; i < 3; ++i)
		{
			if (nsv[i].find(u) != nsv[i].end())
			{
				_nC.emplace(u);
				break;
			}
		}
	}
}
