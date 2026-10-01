#!/usr/bin/python3
# -*- coding: utf-8 -*-


import re
from base_parser import BaseParser
from base_parser import Progress


TYPES = [".edges"]
pattern = r"[ ,\n\t]+"
split_valve = 0.5


class EdgesParser(BaseParser):
	def __init__(self, _src: str, _des: str, _type: str):
		super().__init__(_src, _des)
		self.type = _type

	def parse(self):
		super().parse()
		try:
			TYPES.index(self.type)
		except ValueError:
			print("error: type " + self.type + " is not supported")
		else:
			with open(self.data_src, "r") as f:
				print("loading: " + self.data_src + " start")
				edges = []
				t_mm = [0, 0xffffffff]

				for line in f:
					items = re.split(pattern, line.strip())
					uid = int(items[0])
					vid = int(items[1])
					time = int(float(items[-1]))

					if uid == vid or time == 0:
						continue
					else:
						if self.vm.get(uid, None) is None:
							self.vm[uid] = self.cm
							self.cm = self.cm + 1
						if self.vm.get(vid, None) is None:
							self.vm[vid] = self.cm
							self.cm = self.cm + 1

						if time > t_mm[0]:
							t_mm[0] = time
						if time < t_mm[1]:
							t_mm[1] = time
						edges.append((uid, vid, time))

				edges = sorted(edges, key=lambda v: (min(v[0], v[1]), max(v[0], v[1]), v[2]))
				edges.append((-1, -1, -1))
				print("loading: " + self.data_src + " end")

				print("parsing: " + self.data_src + " start")
				t_valve = t_mm[1] + (t_mm[0] - t_mm[1]) * split_valve
				e = len(edges)
				p = Progress(0, e - 1)
				t_range = None

				for i in range(e):
					p.show(i)

					if i == 0:
						t_range = [edges[i][2], edges[i][2]]
						continue
					elif i > 0 and min(edges[i][0], edges[i][1]) == min(edges[i - 1][0], edges[i - 1][1]) and max(edges[i][0], edges[i][1]) == max(edges[i - 1][0], edges[i - 1][1]):
						t_range[1] = edges[i][2]
						continue
					else:
						uid = min(edges[i - 1][0], edges[i - 1][1])
						vid = max(edges[i - 1][0], edges[i - 1][1])
						if self.cache.get(uid, None) is None:
							self.cache[uid] = [set(), set(), set()]
						if self.cache.get(vid, None) is None:
							self.cache[vid] = [set(), set(), set()]

						ln = 0
						if t_range[0] < t_valve <= t_range[1]:
							ln = 2
						elif t_range[0] >= t_valve:
							ln = 1
						self.cache[uid][ln].add(vid)
						self.cache[vid][ln].add(uid)

						t_range = [edges[i][2], edges[i][2]]
				self.ready = True
				print("parsing: " + self.data_src + " end")
