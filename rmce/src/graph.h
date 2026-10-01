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
			amount
		};
		graph() {};
		void load(const string&);
		void print_info() const;
		void reduction_vertex(double);
		void reduction_edge();
		void reorder(sort_method);
		const subgraph* data() const
		{
			return &this->sg;
		};
		const list* priority() const
		{
			return &this->vpm;
		};
		const list* current_mr() const
		{
			return &this->pmr;
		};
		const st_pair_list* flag() const
		{
			return &this->fl;
		};
	private:
		ST va = 0;
		array<ST, 2ULL> ea = { 0, 0 }, dm = { 0, 0 };
		subgraph sg;
		list vpm, pmr;
		st_pair_list fl;
	};
}
