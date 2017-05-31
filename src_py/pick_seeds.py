import sys
import numpy as np
import pickle

if __name__ == '__main__':
	freq_e=pickle.load(open(sys.argv[1],'rb'))
	freq_r=pickle.load(open(sys.argv[2],'rb'))
	OUT=open(sys.argv[3],'w')
	#relation_tokens=set()
	#random_seeds=int(sys.argv[3])
	#with open(sys.argv[1]) as IN:
	#	for line in IN:
	#		relation_tokens.add(line.strip())
	entity_multigram=set()
	entity_unigram=set()
	for e,v in freq_e.iteritems():
		if e == 1:
			entity_unigram=entity_unigram.union(v)
		else:
			entity_multigram=entity_multigram.union(v)
	relation_multigram=set()
	for r,v in freq_r.iteritems():
		if v>15:
			relation_multigram.add(r)
	num_multigram=2000
	num_unigram=1000
	num_relation=1000
	for e in np.random.choice(list(entity_multigram),num_multigram):
		OUT.write(e+'\n')
	for e in np.random.choice(list(entity_unigram),num_unigram):
		OUT.write(e+'\n')
	for r in np.random.choice(list(relation_multigram),num_relation):
		OUT.write(r+'\n')
	OUT.close()
	 
