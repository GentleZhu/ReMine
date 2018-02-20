import sys
import argparse,json,operator
import numpy as np
from collections import defaultdict

class PostProcessor(object):
	def load_em_emb(self, em_emb_path):
		self.em_emb = dict()
		
		with open(em_emb_path, 'rb') as IN:
			IN.readline()
			for line in IN:
				line=line.strip().split(' ')
				self.em_emb[line[0]] = np.asarray(map(float, line[1:]))
		print("em emb loaded!")
	
	def load_rm_emb(self, rm_emb_path):
		self.rm_emb = dict()

		with open(rm_emb_path, 'rb') as IN:
			IN.readline()
			for line in IN:
				line=line.strip().split(' ')
				self.rm_emb[line[0]] = np.asarray(map(float, line[1:]))
		print("rm emb loaded!")

	def rank_tri(self, train, rank_out):
		candidates = []
		with open(train, 'r') as IN, open(rank_out, 'w') as OUT:
			previous_doc = -1
			for line_num, line in enumerate(IN):
				#print line_num
				tmp = line.strip().split(' ')
				if int(tmp[3]) > previous_doc and len(candidates) > 0:
					previous_doc = int(tmp[3])
					#candidates.sort(key=lambda x: x[1])
					for i in candidates:
						OUT.write(i[0].strip()+'\t'+str(i[1])+'\n')
					candidates = []
				try:
					em1 = self.em_emb[tmp[0]]
					em2 = self.em_emb[tmp[1]]
					rms = tmp[2].strip().split(',')
				#if int(tmp[3]) > 
				
					rm = self.rm_emb[rms[0]]
					for r in rms[1:]:
						rm += self.rm_emb[r]
					rm /= len(rms)
					candidates.append((line, np.linalg.norm(em1+rm-em2, ord=1)))
				except:
					print(line_num)
					#print(rms)
				#if line_num > 30:
				#	break
	def generate_output(self, rank_out, train_json, out):
		with open(rank_out) as IN, open(train_json) as IN_JSON, open(out, 'w') as OUT:
			lines = IN_JSON.readlines()
			print(len(lines))
			for line in IN:

				tmp = line.strip().split('\t')
				score = tmp[1]
				tmp = tmp[0].split(' ')
				docID = tmp[3]
				if int(docID) < 761106:
					continue
				else:
					docID = str(int(docID) - 761105)
				tmp_json = json.loads(lines[int(tmp[4])])
				predicates = '_'.join(tmp_json['tokens'])
				
				#for rp in tmp[2].strip().split(','):
				#	predicates = predicates.replace(rp, '['+rp+']')
				predicates = predicates.replace('_and','')
				
				OUT.write(docID + '\t' + score + '\t' + tmp_json['entityMentions'][0][2] + '\t"' + predicates
					+ '"\t' + tmp_json['entityMentions'][1][2]+'\n')

			
	def compareLineByLine(self, test_file_a, test_file_b, out):
		with open(test_file_a) as IN_A, open(test_file_b) as IN_B, open(out, 'w') as OUT:
			cnt = 1
			for line_a, line_b in zip(IN_A, IN_B):
				pred_a = set()
				pred_b = set()
				for item in line_a.split(']_['):
					if ':EP' in item:
						pred_a.add(item.rstrip(' :EP').strip().replace('(','-lrb-').replace(')','-rrb-'))
					elif ':BP' in item:
						pred_a.add(item.rstrip(' :BP').strip().replace('(','-lrb-').replace(')','-rrb-'))
				for item in line_b.split(']_['):
					if ':EP' in item:
						pred_b.add(item.rstrip(' :EP').strip().replace('(','-lrb-').replace(')','-rrb-'))
					elif ':BP' in item:
						pred_b.add(item.rstrip(' :BP').strip().replace('(','-lrb-').replace(')','-rrb-'))
				OUT.write(str(cnt) + '\t')
				OUT.write('_'.join(list(pred_a - pred_b))+'\t\t')
				OUT.write('_'.join(list(pred_b - pred_a))+'\n')
				cnt += 1
				
	
	def extract(self,test_file,json_file,pos_file,output):
		with open(test_file,'r', encoding='utf-8') as IN, open(json_file, 'r', encoding='utf-8') as IN_JSON, open(pos_file, 'r') as IN_POS, open(output,'w') as OUT:
			e_not_found = 0
			r_not_found = 0
			cnt = 0
			for line, json_line, pos_line in zip(IN, IN_JSON, IN_POS):
				cnt += 1
				pred=[]
				pred_rm = []
				for item in line.split(']_['):
					if ':EP' in item:
						pred.append(item.rstrip(' :EP').strip().replace('(','-lrb-').replace(')','-rrb-'))
					elif ':BP' in item:
						pred.append(item.rstrip(' :BP').strip().replace('(','-lrb-').replace(')','-rrb-'))
				tmp = {}
				tmp['tokens'] = json_line.strip().split(' ')
				#tmp['lemmas'] = 
				tmp['pos'] = pos_line.strip().split(' ')
				#exists = set()
				cur_max = dict()
				tmp['entityMentions'] = []
				ptr = 0
				for e in pred:
					window_size=e.count(' ') + 1
				
					found=False
					#ptr = 0
					while ptr+window_size <= len(tmp['tokens']):
						if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == e:
							found=True
							break
						ptr+=1
					#if found and (ptr, ptr+window_size) not in exists:
					if not found:
						ptr = 0 
						#print(e+"not found")
						e_not_found += 1
					if found:
						if ptr+window_size not in cur_max:
							cur_max[ptr+window_size] = ptr
							#tmp['entityMentions'].append([ptr, ptr+window_size, e])
						ptr+=window_size
				#keys = cur_max.keys().sort(reverse=True)
				ptr = len(tmp['tokens'])
				while ptr > 0:
					if ptr in cur_max:
						tmp['entityMentions'].append([cur_max[ptr], ptr, ' '.join(tmp['tokens'][cur_max[ptr]:ptr])])
						ptr = cur_max[ptr]
					else:
						ptr -= 1
					
				#tmp['entityMentions'] = list(exists)
				tmp['entityMentions'].sort(key=operator.itemgetter(1))
				
				
				new = []
				for i in range(len(tmp['entityMentions'])-1):
					if tmp['entityMentions'][i][1] == tmp['entityMentions'][i+1][0] and (tmp['entityMentions'][i+1][2][0:2] == 'of' or tmp['entityMentions'][i][2][-2:] == 'of' or tmp['entityMentions'][i+1][2][0:2] == "'s" or tmp['entityMentions'][i][2][-2:] == "'s"):
						postags = ''.join(tmp['pos'][tmp['entityMentions'][i][0]:tmp['entityMentions'][i+1][1]])
						if 'NN' in postags or 'W' in postags:
							new.append([tmp['entityMentions'][i][0], tmp['entityMentions'][i+1][1], tmp['entityMentions'][i][2] + ' ' + tmp['entityMentions'][i+1][2]])
							#if cnt >= 235973:
								#print(tmp['entityMentions'][i][2], tmp['entityMentions'][i+1][2])
							#if tmp['']
						#else:
						#	print(tmp['entityMentions'][i][2] + ' ' + tmp['entityMentions'][i+1][2])
						#print(new[-1][2])
					elif len(new) == 0 or tmp['entityMentions'][i][0] >= new[-1][1]:
						postags = ''.join(tmp['pos'][tmp['entityMentions'][i][0]:tmp['entityMentions'][i][1]])
						if 'NN' in postags or 'W' in postags or 'PRP' in postags:
							new.append(tmp['entityMentions'][i])
						#else:
						#	print(tmp['entityMentions'][i][2])
				if len(new) == 0:
					new = tmp['entityMentions']
				elif new[-1][1] != tmp['entityMentions'][-1][1]:
					new.append(tmp['entityMentions'][-1])

				for idx,item in enumerate(new):
					postags = tmp['pos'][item[0]:item[1]]
					start = int(item[0])
					end = int(item[1])
					xxxx = item[2].strip().split(' ')
					if postags[0] == 'IN' or postags[0] == 'CC' or postags[0] == 'TO':
						del xxxx[0]
						start += 1
					if len(xxxx) > 0:
						if postags[-1] == 'IN' or postags[-1] == 'CC' or postags[-1] == 'TO':
							del xxxx[-1]
							end -= 1
					if start != int(item[0]) or end != int(item[1]):
						if start != end:
							new[idx] = [start, end, ' '.join(xxxx)]
							#print(item[2], ' '.join(xxxx))
						else:
							pass
							#print("^^^^^^^"+item[2]+"^^^^^^^")
				tmp['entityMentions'] = new


				OUT.write(json.dumps(tmp) + '\n')
			print("#entity not found:",e_not_found)

	def transformat(self, file_path, output):
		with open(file_path) as IN, open(output, 'w') as OUT:
			for line in IN:
				tmp = json.loads(line)
				ems = ''
				for em in tmp['entityMentions']:
					if len(em[2]) > 0:
						ems += str(em[0]) + '_'  + str(em[1]) + ' '
				OUT.write(ems.strip()+'\n')

	def generatePathwords(self, input_json, input_pair, input_dep, out, out2):
		self.punc = {'.',',','"',"'",'?',':',';','-','!','-lrb-','-rrb-','``',"''", ''}
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
				for item in line_seg.strip().split('<>'):
					dump = {}
					if item == '':
						continue
					annotation = item.strip().split('\t')
					#print annotation

					idx1 = int(annotation[0].split(' ')[0])
					idx2 = int(annotation[0].split(' ')[1])

					if len(annotation) == 1 or len(annotation[1]) == 0:
						'''
						if cnt >= 761105:
							em1 = entityMentions[idx1]
							em2 = entityMentions[idx2]
							if em2[0] - em1[1] <= 2 and em2[0] != em1[1]:
								predicates = ' '.join(tokens[em1[1]:em2[0]])
								if predicates == ',':
									predicates = 'is (at, of)'
								if 'be' in predicates or 'of' in predicates or 'in' in predicates or 'like' in predicates:
									print(str(cnt-761105) + '\t0.0\t'+em1[2]+'\t' + predicates+'\t' +em2[2])
						continue
						'''
						continue
					ranges = list(map(lambda x:int(x)-1, annotation[1].strip().split(' ')))
					
					#there are still possibilities
					if len(ranges) == 1 and tokens[ranges[0]] in self.punc:
						continue
					
					#ranges = range(entityMentions[idx1][0],entityMentions[idx1][1]) +\
					#ranges + range(entityMentions[idx2][0],entityMentions[idx2][1])
					#ranges = map(lambda x:str(x[0])+'_'+x[1],sorted(list(ranges)))
					#print ranges
					
					dump['tokens'] = list(map(lambda x: tokens[x], ranges))
					dump['pos'] = list(map(lambda x: tags[x], ranges))
					#dump['tokens'] = tokens[ranges[0]:ranges[-1]+1]
					dump['doc'] = cnt
					#dump['pos'] = tags[ranges[0]:ranges[-1]+1]
					entityMentions[idx1].append(tags[entityMentions[idx1][0]:entityMentions[idx1][1]])
					entityMentions[idx2].append(tags[entityMentions[idx2][0]:entityMentions[idx2][1]])
					dump['entityMentions'] = [entityMentions[idx1], entityMentions[idx2]]
					for i in ranges:
						fout.write(str(deps[i]) + '\n')
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
			#print(cnt)
		fout.close()

	def combine(self, file_a, file_b, file_out):
		with open(file_a) as IN_A, open(file_b) as IN_B, open(file_out, 'w') as OUT:
			extras = defaultdict(list)
			for line in IN_B:
				docID = line.strip().split('\t')[0]
				extras[docID].append(line)
			for line in IN_A:
				docID = line.strip().split('\t')[0]
				#print(extras[str(docID)])
				#print(docID)
				for s in extras[docID]:
					OUT.write(s)
				del extras[docID]
				OUT.write(line)


	def loadRMTest(self, test_file,json_file,output, out1,out2, out3):
		self.punc = ['.',',','"',"'",'?',':',';','-','!','(',')','``',"''", '']
		print(output)
		ems=set()
		rms=set()
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT, open(out3, 'w') as N_OUT:
			for idx, (line, json_line) in enumerate(zip(IN, IN_JSON)):
				pred=[]
				for item in line.split(']_['):
					if ':RP' in item:
						item = item.rstrip(' :RP').lower().replace(' ', '_')
						if item not in self.punc:
							rms.add(item)
					#if ' ' in item:
					#if ' ' in item.strip():
							pred.append(item)
				tmp_json = json.loads(json_line)
				pos_segs = ''.join(tmp_json['pos'])
				if ('VB' in pos_segs or 'IN' in pos_segs):
					tmp = tmp_json['entityMentions']
					em_1 = tmp[0][2].lower().replace("''",'').replace(' ', '_')
					em_2 = tmp[1][2].lower().replace("''",'').replace(' ', '_')
					if len(em_1) == 0 or len(em_2) == 0:
						continue
					ems.add(em_1)
					ems.add(em_2)
					if len(pred) > 0:
						OUT.write(em_1 +' '+ em_2 + ' '+','.join(pred) + ' ' + str(tmp_json['doc'])+ ' ' + str(idx) + '\n')
					#elif tmp_json['doc'] >= 235973:
					#	tmp = tmp_json['entityMentions']
						#print(str(cnt-235972) + '\t0.0\t'+em1[2]+'\t' + predicates+'\t' +em2[2])
					#	print(str(tmp_json['doc']-235972) + '\t0.0\t' + tmp[0][2] + '\t' + '_'.join(tmp_json['tokens'])+'\t'+tmp[1][2])
				else:
					N_OUT.write(json_line)
		with open(out1, 'w') as w1, open(out2, 'w') as w2:
			for i in list(ems):
				w1.write(i+'\n')
			for i in list(rms):
				w2.write(i+'\n')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description="Run node2vec.")
	parser.add_argument('--in1', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')
	parser.add_argument('--in2', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')
	parser.add_argument('--in3', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')

	parser.add_argument('--out1', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')
	parser.add_argument('--out2', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')
	parser.add_argument('--out3', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')
	parser.add_argument('--out4', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')

	parser.add_argument('--op', help='Type of supervision')

	args = parser.parse_args()
	tmp = PostProcessor()
	if args.op == 'extract':
		tmp.extract(args.in1, args.in2, args.in3, args.out1)
	elif args.op == 'transformat':
		tmp.transformat(args.in1, args.out1)
	elif args.op == 'generatepath':
		tmp.generatePathwords(args.in1, args.in2, args.in3, args.out1, args.out2)
	elif args.op == 'generatetri':
		tmp.loadRMTest(args.in1, args.in2, args.out1, args.out2, args.out3, args.out4)
	elif args.op == 'ranktri':
		tmp.load_em_emb(args.in1)
		tmp.load_rm_emb(args.in2)
		tmp.rank_tri(args.in3, args.out1)
	elif args.op == 'study1':
		tmp.compareLineByLine(args.in1, args.in2, args.out1)
	elif args.op == 'generateoutput':
		tmp.generate_output(args.in1, args.in2, args.out1)
	elif args.op == 'combine':
		tmp.combine(args.in1, args.in2, args.out1)