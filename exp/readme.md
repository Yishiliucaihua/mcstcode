## Preparation

### datasets
<ol>
 <li>each line in raw files (with suffix ".edge") represents an edge e=(u,v,t), where u,v are two vertices and t is the timestamp</li>
 <li>create directories for raw files (<i>src_dir</i>) and generated files (<i>dst_dir</i>)</li>
 <li>move raw files to <i>src_dir</i>, run "parse_edges.py", and the datasets can be used are in <i>dst_dir</i></li>
</ol>

### before running
move all executable files, i.e., mcst_naive, mcst_normal and mcst_imp, into this folder

## Conduct experiments

in "run_exp.py", set needed cmd

``
python3 ./run_exp.py
``