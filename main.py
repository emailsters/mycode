#! /usr/bin/python
#-*-coding:utf-8-*-
import os
import sys

g_test_cases = ('test1', 'test2', 'test3')
g_cases_home = os.getcwd()

def compile_action(flag):
	os.chdir(g_cases_home)

	if flag == 0:
		ci_cmd = 'make'
	elif flag == 1:
		ci_cmd = 'make clean'

	for case_dir in g_test_cases:
		cdir = os.path.join('/', g_cases_home, case_dir)
		os.chdir(cdir)
		print('Entering directary:%s' %(cdir))
		os.system(ci_cmd)

if __name__ == '__main__':
	arg_num = len(sys.argv);
	if arg_num > 1:
		arg = sys.argv[1]
		if arg == '-b':
			compile_action(0)
		elif arg == '-c':
			compile_action(1)
	else:
		print('use -b to compile, -c to clean')
