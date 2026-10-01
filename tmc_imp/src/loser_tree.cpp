#include "./loser_tree.h"

using namespace core;

loser_tree::loser_tree(ST _size)
{
	if (_size < 2)
	{
		return;
	}

	this->ls.resize(_size, _size);
	this->data.resize((ULL)_size + 1, -1);
	this->offset = _size;
}

void loser_tree::update_item(ST _tid, ST _value)
{
	if (this->offset == -1)
	{
		return;
	}

	this->data[_tid] = _value;
	ST ts = (_tid + this->offset) >> 1;
	
	while (ts > 0)
	{
		if (this->data[_tid] > this->data[this->ls[ts]])
		{
			this->ls[ts] ^= _tid;
			_tid ^= this->ls[ts];
			this->ls[ts] ^= _tid;
		}
		ts >>= 1;
	}
	this->ls[0] = _tid;
}

ST loser_tree::top(ST &_tid) const
{
	if (this->offset == -1)
	{
		return ST_INVALID;
	}

	_tid = this->ls[0];
	return this->data[_tid];
}
