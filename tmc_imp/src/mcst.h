#pragma once

#include <cmath>
#include "./search_interface.h"
#include "./loser_tree.h"

using std::copy;
using std::max;
using std::ceil;
using std::log2;

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
		unordered_map<ST, degrees> R, tempR, deltaC;
		ST root_pos = -1;
		sd_list C, tempSD;
		set tempS, tempS1, tempS2, tempS3;
		list tempL, im, v2i;
		unordered_map<ST, ST> mapping;
		degrees ts = { 0, 0, 0 };
		void gen_subgraph(const subgraph&, subgraph&, ST);
		void construct(ST);
		void solve_imp(ST, bool, ST);
		bool check() const;
		void updateR(ST, ST);
		void restoreR(ST);
		bool cost_cmp(const neighbors&, ST);
		void markC(ST, ST, ST);
		void collect(ST);
		void generateNC(ST, ST&);
		void restoreNC(ST, ST);
		void restoreC(ST, ST);
		void restoreS(ST, ST);
		ST intersection_cnt_pivot(ST, ST, double&);
		ST pivot(ST, ST);
		ST intersection_cnt_prune(ST, ST, ST);
		void prune(ST);
		bool intersection_adv_prune_R(const sd_list&, ST, ST, ST, ST);
		bool intersection_adv_prune_C(const sd_list&, ST, ST, ST, ST);
		bool adv_prune(ST, ST&);
		bool intersection_verify(ST);
		bool verify(ST);
	};
}
