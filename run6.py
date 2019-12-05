import os

path = "instances/"

for file in os.listdir(path):
	command = "./MO420_Branch_and_Cut 00100 " + path + file
	print(command)
	os.system(command)