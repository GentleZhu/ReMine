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
	
	def loadTest(self,test_file,json_file,output):
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT:
			for line, json_line in zip(IN, IN_JSON):
				pred=[]
				for item in line.split(','):
					if ':EP' in item:
						item = item.rstrip(' :EP')
					#if ' ' in item:
					#if ' ' in item.strip():
						pred.append(item)
				tmp = json.loads(json_line)
				#exists = set()
				cur_max = dict()
				for i in tmp['entityMentions']:
					#exists.add((i[0], i[1]))
					cur_max[i[1]] = i[0]

				tmp['entityMentions'] = []
				for e in pred:
					window_size=e.count(' ') + 1
				
					found=False
					ptr = 0
					while ptr+window_size <= len(tmp['tokens']):
						ptr+=1
						if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == e:
							found=True
							break
					#if found and (ptr, ptr+window_size) not in exists:
					if found:
						if ptr+window_size in cur_max and ptr < cur_max[ptr+window_size]:
							cur_max[ptr+window_size] = ptr
						elif ptr+window_size not in cur_max:
							tmp['entityMentions'].append([ptr, ptr+window_size, e])
				for k,v in cur_max.iteritems():
					tmp['entityMentions'].append([v, k, ' '.join(tmp['tokens'][v:k])])
				#tmp['entityMentions'] = list(exists)
				tmp['entityMentions'].sort(key=operator.itemgetter(1))
				OUT.write(json.dumps(tmp) + '\n')

	def loadRMTest(self, test_file,json_file,output):
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT:
			for line, json_line in zip(IN, IN_JSON):
				pred=[]
				for item in line.split(','):
					if ':EP' in item:
						item = item.rstrip(' :EP')
					#if ' ' in item:
					#if ' ' in item.strip():
						pred.append(item)
				tmp = json.loads(json_line)
				tmp['relationMentions'] = []
				for e in pred:
					window_size=e.count(' ') + 1
				
					found=False
					ptr = 0
					while ptr+window_size <= len(tmp['tokens']):
						ptr+=1
						if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == e:
							found=True
							break
					#if found and (ptr, ptr+window_size) not in exists:
					if found:
						tmp['relationMentions'].append([ptr, ptr+window_size, e])
				tmp['relationMentions'].sort(key=operator.itemgetter(1))
				OUT.write(json.dumps(tmp) + '\n')
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
	#tmp.loadTest(sys.argv[1], sys.argv[2], sys.argv[3])
	tmp.loadRMTest(sys.argv[1], sys.argv[2], sys.argv[3])
	#tmp.loadNER(sys.argv[2])
	#tmp.getMetrics()