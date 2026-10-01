#!/usr/bin/python3
# -*- coding: utf-8 -*-


from edges_parser import EdgesParser
from base_parser import get_file_list


src_dir = "src_dir"
dst_dir = "dst_dir"

fp_l = get_file_list(src_dir, dst_dir, ".edges")
for fp in fp_l:
	gp = EdgesParser(fp[0], fp[1], ".edges")
	gp.parse()
	gp.store()
