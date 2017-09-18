import numpy as np
import json,sys

class Extractor(object):
	"""docstring for Extractor"""
	def __init__(self, arg):
		self.arg = arg
		self.emb = dict()

	def load_emb(self):
		with open(self.arg['emb_path']) as IN:
			IN.readline()
			for line in IN:
				line=line.strip()
				self.emb[line.split(' ')[0]] = np.asarray(map(float, line.split(' ')[1:]))

	def generateSegments(self, input_json, out1, out2):
		with open(input_json, 'r') as IN, open(out1, 'w') as OUT, open(out2, 'w') as OUTT:
			for line in IN:
				tmp = json.loads(line)
				em = tmp['entityMentions']
				for i in em:
					for j in em[:-1]:
						if i[1] < j[0]:
							new_line = {}
							new_line['tokens'] = tmp['tokens'][i[1]:j[0]]
							new_line['tokens'].append('.')
							new_line['pos'] = tmp['pos'][i[1]:j[0]]
							new_line['pos'].append('.')
							OUT.write(json.dumps(new_line)+'\n')
							new_line = {}
							new_line['em1'] = i[2]
							new_line['em2'] = j[2]
							OUTT.write(json.dumps(new_line)+'\n')
							break
	'''
	def combine(self, dep_1, dep_2, em1, em2):
		if ' ' in em1[2] and ' ' in em2[2]:
			return False

	
	def combineSegments(self, input_json, input_deps, out):
		with open(input_json, 'r') as IN_JSON, open(input_deps,'r') as IN_DEPS, open(out,'w') as OUT:
			for line, deps in zip(IN_JSON, IN_DEPS):
				tmp = json.loads(line)
				dep = map(lambda x: (x.split('_')[0], x.split('_')[1]),deps.split(' '))
				assert(len(dep) == tmp['tokens'])
				em = tmp['entityMentions']
				start_index = dict()
				map(lambda x: start_index[x[1][0]] = x[0], enumerate(em))
				empty_slots = []
				for i in em:
					if i[1] in start_index:
						if combine(dep[i[0]:i[1]], dep[], i, em[])
	'''
	
	def generateDepSegmentsv2(self, input_json, input_pair, out):
		with open(input_json, 'r') as IN, open(input_pair, 'r') as SEG, open(out, 'w') as OUT:
			for line, line_seg in zip(IN, SEG):
				new_line = {}
				new_line['triples'] = []
				tmp = json.loads(line)
				rm_indices = dict()
				for rm in tmp['relationMentions']:
					rm_indices[(rm[0], rm[1] - 1)] = rm[2]
				#print rm_indices
				em = tmp['entityMentions']
				for item in line_seg.split('<>'):
					annotation = item.split('\t')
					#print annotation
					if len(annotation) == 1 or len(annotation[1]) == 0:
						continue
					indices = annotation[0].strip().split(' ')
					ranges = set(map(lambda x:int(x) - 1, annotation[1].strip().split(' ')))
					#print ranges
					rms = []
					for k,v in rm_indices.iteritems():
						if k[0] in ranges and k[1] in ranges:
							rms.append(v)
					if len(rms) > 0:
						new_line['triples'].append({'em1':em[int(indices[0])], 'em2':em[int(indices[1])], 'rm':rms})
				#print json.dumps(new_line)
				OUT.write(json.dumps(new_line) + '\n')
				#break
	
	def generateDepSegmentsv1(self, input_json, input_pair, out):
		with open(input_json, 'r') as IN, open(input_pair, 'r') as SEG, open(out, 'w') as OUT:
			for line, line_seg in zip(IN, SEG):
				new_line = {}
				new_line['triples'] = []
				tmp = json.loads(line)
				rm_indices = dict()
				for rm in tmp['relationMentions']:
					rm_indices[(rm[0], rm[1] - 1)] = rm[2]
				#print rm_indices
				em = tmp['entityMentions']
				for item in line_seg.split('<>'):
					annotation = item.split('\t')
					#print annotation
					if len(annotation) == 1 or len(annotation[1]) == 0:
						continue
					indices = annotation[0].strip().split(' ')
					ranges = (em[int(indices[0])][1], em[int(indices[1])][0])
					#ranges = set(map(lambda x:int(x) - 1, annotation[1].strip().split(' ')))
					#print ranges
					rms = []
					for k,v in rm_indices.iteritems():
						#if k[0] in ranges and k[1] in ranges:
						if k[0] >= ranges[0] and k[1] < ranges[1]:
							rms.append(v)
					if len(rms) > 0:
						new_line['triples'].append([em[int(indices[0])][2], ','.join(rms) ,em[int(indices[1])][2]])
				#print json.dumps(new_line)
				OUT.write(json.dumps(new_line) + '\n')
				#break
				
	def generatePathwords(self, input_json, input_pair, input_dep, out, out2):
		fout = open(out2, 'w')
		with open(input_json, 'r') as IN, open(input_pair, 'r') as SEG, open(input_dep, 'r') as DEP, open(out, 'w') as OUT:
			cnt = 1
			for line, line_seg, line_dep in zip(IN, SEG, DEP):
				tmp = json.loads(line)
				deps = line_dep.strip().split(' ')
				rm_indices = dict()
				
				#for rm in tmp['relationMentions']:
				#	rm_indices[(rm[0], rm[1] - 1)] = rm[2]
				#print rm_indices
				tokens = tmp['tokens']
				tags = tmp['pos']
				entityMentions = tmp['entityMentions']
				for item in line_seg.split('<>'):
					dump = {}
					annotation = item.split('\t')
					#print annotation
					if len(annotation) == 1 or len(annotation[1]) == 0:
						continue
					
					#indices = annotation[0].strip().split(' ')
					#annotation[1].split('')
					#ranges = (em[int(indices[0])][1], em[int(indices[1])][0])
					#print annotation[1]
					ranges = map(lambda x:int(x)-1, annotation[1].strip().split(' '))
					idx1 = int(annotation[0].split(' ')[0])
					idx2 = int(annotation[0].split(' ')[1])
					ranges = range(entityMentions[idx1][0],entityMentions[idx1][1]) +\
					ranges + range(entityMentions[idx2][0],entityMentions[idx2][1])
					#ranges = map(lambda x:str(x[0])+'_'+x[1],sorted(list(ranges)))
					#print ranges
					dump['tokens'] = map(lambda x: tokens[x], ranges)
					dump['pos'] = map(lambda x: tags[x], ranges)
					dump['entityMentions'] = [entityMentions[idx1], entityMentions[idx2]]
					for i in ranges:
						fout.write(str(i)+'_'+str(deps[i]) + '\n')
					OUT.write(json.dumps(dump) + '\n')
					#OUT.write(entityMentions[idx1][2] + ' ')
					#OUT.write(' '.join(ranges) + ' ' + entityMentions[idx2][2] + '\n')
				cnt += 1
				#if cnt > 10:
				#	break	
				#return
					#OUT.write()
				#print json.dumps(new_line)
				#OUT.write(json.dumps(new_line) + '\n')
			print(cnt)
		fout.close()
	
	def generateRPs(self, in1, in2, out):
		with open(in1) as IN1, open(in2) as IN2, open(out, 'w') as OUT:
			for line1, line2 in zip(IN1, IN2):
				tmp = json.loads(line2)
				tmp['rm'] = []
				for x in line1.split(','):
					if ':EP' in x:
						tmp['rm'].append(x.rstrip(' :EP'))
				OUT.write(json.dumps(tmp) + '\n')

	
	def load_corpus(self):
		cnt=0
		with open(self.arg['corpus_path']) as IN:
			for line in IN:
				cnt+=1
				if cnt>100:
					break
				sent=[]
				for phrase in line.split(','):
					if ':EP' in phrase:
						sent.append((phrase.rstrip(':EP').strip().replace(' ','_').lower(),'E'))
					elif ':RP' in phrase:
						sent.append((phrase.rstrip(':RP').strip().replace(' ','_').lower(),'R'))
				print sent
				self.process(sent)
		
	def process(self, sent):
		window_size=10
		
		for i,seg in enumerate(sent):
			if seg[1] == 'R':
				scores=[]
				head=[]
				tail=[]
				for h in range(max(0,i-window_size),i):
					if sent[h][1] == 'E':
						head.append(sent[h][0])
				for t in range(i+1,min(len(sent),i+window_size)):
					if sent[t][1] == 'E':
						tail.append(sent[t][0])
				for h in head:
					for t in tail:
						if h not in self.emb or t not in self.emb:
							continue
						scores.append({'head':h, 'r':seg[0], 'tail':t,
							'score':np.linalg.norm(self.emb[h]-self.emb[seg[0]])+
							np.linalg.norm(self.emb[t]-self.emb[seg[0]])})
				print head,tail
				print scores

if __name__ == '__main__':
	tmp = Extractor('')
	#tmp.generateSegments(sys.argv[1], sys.argv[2], sys.argv[3])
	#tmp.generateRPs(sys.argv[1], sys.argv[2], sys.argv[3])
	tmp.generatePathwords(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
	#tmp=Extractor({'emb_path':'/shared/data/qiz3/ReMine/utils/word2vec/data/demo.txt.bin',
	#	'corpus_path':'/shared/data/qiz3/ReMine/results_remine/sample.txt'})
	#tmp.load_emb()
	#print "emb loaded"
	#print tmp.emb['born']
	#tmp.load_corpus()