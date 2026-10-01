#include "./mcst.h"

using core::ST;
using core::ULL;
using core::graph;
using core::info_reporter;
using core::search_interface;
using core::mcst;
using core::to_string;

constexpr int parameter_num = 4;

int main(int _argc, char *_argv[])
{
	if (_argc < parameter_num)
	{
		cout << "error: miss necessary parameters" << endl;
		exit(-1);
	}
	
	info_reporter ir;
	ULL sort_time = 0, run_time = 0;
	double theta = strtod(_argv[3], nullptr);

	// load
	graph g;
	g.load(_argv[1]);
	cout << "original graph:=>{" << endl;
	g.print_info();
	cout << "}" << endl;

#ifdef mem
	ir.print_mem("graph", true);
#endif

	// global reduction & reorder
	ir.clear();
	g.reduction_vertex(theta);
	g.reduction_edge();
	g.reorder(graph::sort_method(atoi(_argv[2])));
	sort_time = ir.get_time();
	
	search_interface *si = new mcst(&ir);

	// running
	if (g.data()->size())
	{
		ir.clear();
		si->solve(g, theta);
		run_time = ir.get_time();
	}

	auto mr = si->result();
	cout << "result size:=>{" << mr.size() << "}" << endl;
#ifdef debug
	stable_sort(mr.begin(), mr.end());
	cout << to_string(mr) << endl;
#endif
	cout << "time info:=>{" << sort_time << "," << run_time << "," << sort_time + run_time << "}" << endl;

#ifdef debug
	si->print_mr(*g.data(), mr);
#endif
	return 0;
}
