#pragma once

#include "./utils.h"

using std::stable_sort;

namespace core
{
	class graph
	{
	public:
		enum class sort_method
		{
			none = 0,
			degree_o,
			core_o,
			opt_o,
			amount
		};
		graph() {};
		void load(const string&);
		void print_info() const;
		void reorder(sort_method);
		sort_method method() const
		{
			return this->sm;
		};
		const subgraph* data() const
		{
			return &this->sg;
		};
		const sl_pair_list* priority_value() const
		{
			return &this->pl;
		};
#ifdef debug
		const list* im() const
		{
			return &this->ivpm;
		};
#endif
	private:
		ST va = 0;
		array<ST, 2ULL> ea = { 0, 0 }, dm = { 0, 0 };
		sort_method sm = sort_method::none;
		subgraph sg;
		sl_pair_list pl;
#ifdef debug
		list ivpm;
#endif
	};
}
