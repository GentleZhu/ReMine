import json
import sys
import re, operator

class PostProcessor(object):
	"""docstring for PreProcessor"""
	def __init__(self):
		self.prediction=[]
		self.groundtruth=[]
		self.entity_regex=re.compile("<phrase>(.+?)</phrase>")
		self.ner_regex=re.compile("<.+?>(.*?)<\/.+?>")
		self.punc = ['.',',','"',"'",'?',':',';','-','!','(',')','``',"''", '']
	
	def combineTest(self, ner_json_a, json_file, output):
		with open(ner_json_a) as NER, open(json_file) as IN, open(output, 'w') as OUT:
			for ner,pm in zip(NER, IN):
				cur_max = dict()
				tmp = json.loads(ner)
				tmp_2 = json.loads(pm)
				assert(len(tmp['tokens']) == len(tmp_2['tokens']))
				cur_max = dict()
				for i in tmp['entity_mentions']:
					if i[2] == "PERSON":
						cur_max[i[0]] = i[1]
				for i in tmp_2['entityMentions']:
					if i[0] not in cur_max or i[1] > cur_max[i[0]]:
						cur_max[i[0]] = i[1]
				ptr = 0
				tmp_2['entityMentions'] = []
				while ptr < len(tmp_2['tokens']):
					if ptr in cur_max:
						tmp_2['entityMentions'].append([ptr, cur_max[ptr],' '.join(tmp['tokens'][ptr:cur_max[ptr]])])
						ptr = cur_max[ptr]
					else:
						ptr += 1
				OUT.write(json.dumps(tmp_2) + '\n')

	def loadTest(self,test_file,json_file,output):
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT:
			e_not_found = 0
			r_not_found = 0
			for line, json_line in zip(IN, IN_JSON):
				pred=[]
				pred_rm = []
				for item in line.split(']_['):
					if ':EP' in item:
						pred.append(item.rstrip(' :EP').strip().replace('(','-LRB-').replace(')','-RRB-'))
					#elif ':RP' in item:
					#	pred_rm.append(item.rstrip(' :RP').strip().replace('(','-LRB-').replace(')','-RRB-'))

					#if ' ' in item:
					#if ' ' in item.strip():
						#pred.append(item)
				tmp = {}
				tmp['tokens'] = json_line.strip().split(' ')
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
				'''
				tmp['relationMentions']= []
				ptr = 0
				for r in pred_rm:
					window_size=r.count(' ') + 1
				
					found=False
					#ptr = 0
					while ptr+window_size <= len(tmp['tokens']):
						if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == r:
							found=True
							break
						ptr+=1
					#if found and (ptr, ptr+window_size) not in exists:
					if not found:
						ptr = 0 
						r_not_found += 1
					if found:
						
						tmp['relationMentions'].append([ptr, ptr+window_size, r])
						ptr+=window_size
				tmp['relationMentions'].sort(key=operator.itemgetter(1))
				'''
				OUT.write(json.dumps(tmp) + '\n')
			print e_not_found,r_not_found

	def loadRMTest(self, test_file,json_file,output, out1,out2):
		print(output)
		ems=dict()
		rms=dict()
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT:
			for line, json_line in zip(IN, IN_JSON):
				pred=[]
				for item in line.split(']_['):
					if ':RP' in item:
						item = item.rstrip(' :RP').lower().replace(' ', '_')
						if item not in self.punc:
							rms.add(item)
					#if ' ' in item:
					#if ' ' in item.strip():
							pred.append(item)
				if len(pred) > 0:
					tmp = json.loads(json_line)['entityMentions']
					em_1 = tmp[0][2].lower().replace(' ', '_')
					em_2 = tmp[1][2].lower().replace(' ', '_')
					ems.add(em_1)
					ems.add(em_2)
					OUT.write(em_1 +' '+ em_2 + ' '+','.join(pred) + '\n')
		with open(out1, 'w') as w1, open(out2, 'w') as w2:
			for i in list(ems):
				w1.write(i+'\n')
			for i in list(rms):
				w2.write(i+'\n')
		#print self.prediction
	def loadNER(self,test_file):
		with open(test_file,'r') as IN:
			for line in IN:
				pred=[]
				for item in re.findall(self.ner_regex,line):
					#if ' ' in item:
					pred.append(item.strip().lower())
				self.prediction.append(pred)
		#print self.prediction

	def loadGroundTruth(self, groundtruth_file):
		with open(groundtruth_file,'r') as IN:
			for line in IN:
				gt=[]
				content=json.loads(line)
				for e in content['entityMentions']:
					#if e[1]-e[0]>0:
					#gt.append(' '.join(content['tokens'][e['end']:e['start']]).strip())
					#e[0]-1 in kbp wenqi's run
					gt.append(' '.join(content['tokens'][e[0]-1:e[1]]).strip())
				self.groundtruth.append(gt)
		#
		#print self.groundtruth

	def getMetrics(self):
		true_pos=0.0
		all_pos=0
		all_pred=0
		veri_pos=0
		veri_pred=0
		for i,j in zip(self.groundtruth,self.prediction):
			all_pos+=len(i)
			all_pred+=len(j)
			#veri_pos+=len(set(i))
			#veri_pred+=len(set(j))
			true_pos+=len(set(i)&set(j))
		#print true_pos,all_pos,all_pred #veri_pred,veri_pos
		print true_pos/all_pos,true_pos/all_pred

		
if __name__ == '__main__':
	tmp=PostProcessor()
	#tmp.loadGroundTruth(sys.argv[1])
	#tmp.combineTest(sys.argv[1], sys.argv[2], sys.argv[3])
	tmp.loadTest(sys.argv[1], sys.argv[2], sys.argv[3])
	#tmp.loadRMTest(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
	#tmp.loadNER(sys.argv[2])
	#tmp.getMetrics()