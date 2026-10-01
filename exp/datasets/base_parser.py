#!/usr/bin/python3
# -*- coding: utf-8 -*-


import sys
import os


class BaseParser:
	def __init__(self, _src: str, _des: str):
		self.data_src = _src
		self.data_des = _des
		self.ready = False
		self.cache = {}
		self.vm = {}
		self.cm = 0

	def parse(self):
		self.ready = False

	def store(self):
		if self.ready:
			with open(self.data_des, "w") as f:
				print("storing: " + self.data_des + " start")
				print("vertices %d" % self.cm)
				p = Progress(0, self.cm - 1)

				# reorder
				tg = []
				for i in range(self.cm):
					tg.append([set(), set(), set()])

				for u, ns in self.cache.items():
					nu = self.vm.get(u)
					for v0 in ns[0]:
						nv0 = self.vm.get(v0)
						tg[nu][0].add(nv0)
					for v1 in ns[1]:
						nv1 = self.vm.get(v1)
						tg[nu][1].add(nv1)
					for v2 in ns[2]:
						nv2 = self.vm.get(v2)
						tg[nu][2].add(nv2)

				# store
				for i in range(self.cm):
					p.show(i)

					for k in range(3):
						s = 0
						e = len(tg[i][k])
						for v in tg[i][k]:
							s = s + 1
							if s != e:
								f.write(str(v) + ",")
							else:
								f.write(str(v))

						if k == 2:
							f.write("\n")
						else:
							f.write("|")
				print("storing: " + self.data_des + " end")


class Progress:
	def __init__(self, _min: int, _max: int):
		self.min = _min
		self.max = _max
		self.span = _max - _min + 1
		self.log_span = max(int(self.span * 0.01), 1)

	def show(self, _cur):
		if _cur % self.log_span == 0:
			sys.stdout.write("\rprocessing: {0}%".format(int(round(float(_cur) * 100 / self.span))))
			sys.stdout.flush()
		if _cur == self.max:
			sys.stdout.write("\rprocessing: 100%\n")
			sys.stdout.flush()


def get_file_list(_dir_in: str, _dir_out: str, _suffix=None):
	ret = []
	for root, _, files in os.walk(_dir_in, topdown=False):
		for name in files:
			if _suffix is not None and os.path.splitext(name)[-1] != _suffix:
				continue
			sp = os.path.join(root, name)
			dp = os.path.join(_dir_out, name.replace(".", "_") + ".txt")
			if not os.path.exists(dp):
				ret.append((sp, dp))
	return ret
