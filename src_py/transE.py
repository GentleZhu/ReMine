import sys,json
import cPickle

def load_triples(tri_json, em_out, rm_out, tri_out):
	entities = dict()
	relations = dict()
	with open(tri_json) as tj, open(em_out,'w') as eo, open(rm_out,'w') as ro, open(tri_out,'w') as to:
		for line in tj:
			tmp = json.loads(line)
			for t in tmp['triples']:
				em1 = t[0].lower()
				em2 = t[2].lower()
				rm = t[1].lower()
				if em1 not in entities:
					entities[em1] = 'e_' + str(len(entities))
				if em2 not in entities:
					entities[em2] = 'e_' + str(len(entities))
				if rm not in relations:
					relations[rm] = 'r_' + str(len(relations))
				to.write(entities[em1]+'\t'+entities[em2]+'\t'+relations[rm]+'\n')
		for k,v in entities.iteritems():
			eo.write(v+'\n')
		cPickle.dump(entities, open('entities.p','wb'))
		for k,v in relations.iteritems():
			ro.write(v+'\n')
		cPickle.dump(relations, open('relations.p','wb'))


if __name__ == '__main__':
	load_triples(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])