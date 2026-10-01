#pragma once

#include <cmath>
#include "./search_interface.h"

using std::max;
using std::ceil;

namespace core
{
	class mcst : public search_interface
	{
	public:
		mcst(info_reporter *const _ir, ST _prt) : search_interface(_ir), prt(_prt) {};
		void solve(const graph&, double);
	private:
		ST prt = 0;  // 0: disable adv_prune; 1: enable adv_prune
		subgraph sg;
		set tempS, tempS1, tempS2;
		v_info tempR, tempC, tempI;
		sl_pair_list tempSPL;
		void solve_imp(ST, bool, v_info&, v_info&);
		void gen_subgraph(const subgraph&, subgraph&, ST);
		void construct(ST, v_info&, v_info&) const;
		void updateR(ST, const v_info&, const v_info&, v_info&) const;
		void updateC(ST, const v_info&, v_info&) const;
		bool check(const v_info&) const;
		void prune(ST, v_info&);
		ST intersection_cnt_R(ST, const v_info&, ST, ST);
		ST intersection_cnt_C(ST, const v_info&, ST, ST);
		ST intersection_cnt_P(ST, const v_info&);
		void pivot(ST, const v_info&);
		bool adv_prune(v_info&, v_info&);
		bool verify(v_info&, v_info&);
	};
}
