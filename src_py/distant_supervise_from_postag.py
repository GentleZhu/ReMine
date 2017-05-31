import sys
import numpy as np
import utils

if __name__ == '__main__':
	#relation_tokens=set()
	#with open(sys.argv[1]) as IN:
	#	for line in IN:
	#		relation_tokens.add(line.strip())
	utils.eliminateTab(sys.argv[1],sys.argv[2])