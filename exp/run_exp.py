#!/usr/bin/python3
# -*- coding: utf-8 -*-


import os
import signal
import subprocess
from threading import Timer


is_shell = True
time_out = 36000


def call(_args, _timeout):
	p = subprocess.Popen(_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=is_shell, start_new_session=True)
	timer = Timer(_timeout, lambda process: os.killpg(process.pid, signal.SIGUSR1), [p])
	try:
		timer.start()
		stdout, stderr = p.communicate()
		return_code = p.returncode
		return str(stdout, encoding="utf8"), str(stderr, encoding="utf8"), return_code
	finally:
		timer.cancel()


cmd = "./mcst_naive fp 2 0.7"
print(call(cmd, time_out))
