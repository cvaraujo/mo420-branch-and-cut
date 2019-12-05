import os
import sys

path = "instances/"

for file in os.listdir(path):
	command = "./MO420_Branch_and_Cut 11000 " + path + file
	if not ("_" + sys.argv[1] + "_" in command): continue
	print(command)
	os.system(command)