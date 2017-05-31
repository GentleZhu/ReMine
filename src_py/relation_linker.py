import sys
from collections import defaultdict
import json
class relation_linker(object):
	"""docstring for relation_linker"""
	def __init__(self, fp_path,train_path,lf_path):
		self.relation_pool=defaultdict(int)
		self.file_path=fp_path
		self.train_path=train_path
		self.lf_path=lf_path
	def generatePool(self,min_sup):
		with open(self.file_path,'r') as IN:
			for line in IN:
				tmp=line.strip().split('\t')
				if int(tmp[1])>=min_sup:
					self.relation_pool[tmp[2]]=int(tmp[1])
	def sliding_window(self,min_len,max_len):
		#self.relation_type=defaultdict(int)
		with open(self.train_path,'r') as IN, open('./tmp_remine/relation_token.txt','w') as rt,open('./tmp_remine/pos_relation_token.txt','w') as rp:
			cnt=0
			for line in IN:
				cnt+=1
				tmp=json.loads(line.strip())
				for rm in tmp['relationMentions']:
					if rm[2]=='None':
						continue
					start=rm[0][1]
					end=rm[1][0]
					assert(len(tmp['tokens'])==len(tmp['pos']))
					sub=tmp['tokens'][start:end]
					sub_pos=tmp['pos'][start:end]
					#print sub
					for i in range(min_len,max_len+1):
						for j in xrange(end-start+1-i):
							#print j,j+i
							candidate=' '.join(sub[j:j+i])
							if candidate in self.relationlist:
								for w in sub[j:j+i]:
									rt.write(self.token_mapping[w]+' ')
									#rt.write(w+' ')
								rp.write(' '.join(sub_pos[j:j+i])+'\n')
								rt.write('\n')
								#print candidate,'\t',' '.join(tmp['tokens'])
								#self.relation_type[(rm[2],candidate)]+=1
				#if cnt>10000:
				#	break	
		#for k,v in self.relation_type.iteritems():
		#	if v>10:
		#		print k,v
	def readLF(self):
		self.relationlist=set()
		with open(self.lf_path,'r') as IN:
			for line in IN:
				tmp=json.loads(line.strip())
				candidate=tmp['Texture'].replace('<JJ> ','')
				#can be simplified by comment next line
				#if candidate in self.relation_pool:
				self.relationlist.add(candidate)

	def loadMapping(self,token_mapping):
		self.token_mapping=dict()
		with open(token_mapping,'r') as IN:
			for line in IN:
				tmp=line.strip().split('\t')
				self.token_mapping[tmp[1]]=tmp[0]


		

if __name__ == '__main__':
	tmp=relation_linker(sys.argv[1],sys.argv[2],sys.argv[3])
	#tmp.generatePool(int(sys.argv[4]))
	tmp.readLF()
	tmp.loadMapping('./tmp_remine/token_mapping.txt')
	tmp.sliding_window(2,4)