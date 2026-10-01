#pragma once

#include "./utils.h"

namespace core
{
	class loser_tree
	{
	public:
		loser_tree(ST);
		void update_item(ST, ST);
		ST top(ST&) const;
	private:
		list ls;
		list data;
		ST offset = -1;
	};
}
