#pragma once

#include <cmath>
#include "./search_interface.h"

using std::max;
using std::min;
using std::ceil;

namespace core
{
	class mcst : public search_interface
	{
	public:
		mcst(info_reporter *const _ir) : search_interface(_ir) {};
		void solve(const graph&, double);
	private:
		list ignore;
		set tempS1, tempS2, tempS3;
		unordered_map<ST, ST> tempM;
		void solve_imp(const set&, const set&, const set&);
		void dynamic_prune(set&, set&, set&);
		bool contain(ST, const set&, ST) const;
		void pruneX(ST, const set&, const set&, set&);
		ST intersection_pivot(ST, const set&);
		void pivot(const set&, const set&);
		void create_sub(const set&);
		void solve_sub(const v_info&, const set&);
		bool check(const v_info&) const;
		void updateCX(ST, const set&, set&) const;
	};
}
