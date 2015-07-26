#! /usr/bin/python
#-*-coding:utf-8-*-
import os
import sys
import ConfigParser

g_test_cases = ('test1', 'test2', 'test3')
g_cases_home = os.getcwd()


class Configuration():
    def __init__(self, conf_file):
        self.conf_file = conf_file
        self.conf_parser = ConfigParser.ConfigParser()
        self.conf_parser.read(self.conf_file)

    def get(self, section, key):
        return self.conf_parser.get(section, key, "")
        

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

def fill_template():
    config = Configuration("py_test.cfg")
    tpl_name = config.get("filename", "template")
    file_name = config.get("filename", "target")

    host = config.get("dbparam", "host")
    password = config.get("dbparam", "password")
    user = config.get("dbparam", "user")

    tpl_fp = open(tpl_name, "r")
    file_fp = open(file_name, "w")
    lines = tpl_fp.readlines()

    for line in lines:
        if "__host__" in line:
            file_fp.write(line.replace("__host__", host))
        elif "__password__" in line:
            file_fp.write(line.replace("__password__", password))
        elif "__user__" in line:
            file_fp.write(line.replace("__user__", user))
        else:
            file_fp.write(line)
    tpl_fp.close()
    file_fp.close()

def main(sys):
    arg_num = len(sys.argv);
    if arg_num > 1:
        arg = sys.argv[1]
        if arg == '-b':
            compile_action(0)
        elif arg == '-c':
            compile_action(1)
    else:
        print('use -b to compile, -c to clean')

if __name__ == '__main__':
    #main(sys)
    fill_template()
