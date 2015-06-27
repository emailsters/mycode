#! /usr/bin/python
import os
import logging

logging.basicConfig(
	filename = os.path.join(os.getcwd(), 'log.txt'), 
	level = logging.DEBUG,
	format = '%(asctime)s[%(levelname)s]:%(message)s')

def path_test():
	lpath = os.getcwd()
	print(lpath)
	dirs = os.listdir(lpath)
	print dirs
	if 'test1' in dirs:
		os.chdir('test1')
		os.system('make')

def RemoteCompile(remoteip, account, key, script):
	remotecmd = './ssh_remote.sh ' + remoteip + ' ' + account + ' ' + key + ' ' + script
	ret = os.system(remotecmd)
	logging.info("remote cmd return %d" % (ret))

def main():
	remoteip = '192.168.137.129'
	account = 'youraccount'
	key = 'yourpassword'
	script = "'cd ~/code/cat/cat;./build.sh -c;./build.sh -b'"
	RemoteCompile(remoteip, account, key, script)

if __name__ == '__main__':
	#path_test()
	main()