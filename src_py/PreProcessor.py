import json
import sys
import cPickle
import re
import codecs

class PreProcessor(object):
	"""docstring for PreProcessor"""
	def __init__(self, train_path):
		self.word_mapping = dict()
		self.punc_mapping = dict()
		#self.reverse_mapping=dict()
		self.word_cnt = 1
		self.cache = []
		self.punc = ['.',',','"',"'",'?',':',';','-','!','(',')','``',"''", '']
		self.mode = "Length Penalty Mode"
		if train_path:
			with open(train_path,'r') as IN:
				for line in IN:
					content=json.loads(line)
					self.cache.append(content)


	def inWordmapping(self,word):
		if word in self.punc and self.mode != 'Constraints Mode':
			return word
		if word not in self.word_mapping:
			self.word_mapping[word]=self.word_cnt
			if word in self.punc:
				self.punc_mapping[self.word_cnt] = word
			#self.reverse_mapping[self.word_cnt]=word
			self.word_cnt+=1
		return str(self.word_mapping[word])

	def inTrainmapping(self,word):
		if word in self.punc and self.mode != 'Constraints Mode':
			return word
		if word in self.word_mapping:
			return str(self.word_mapping[word])
		else:
			return '-1111'
	
	def tokenize_train(self,out_path):
		CASE=open('case_'+out_path,'w')
		POS_tag=open('pos_tags_tokenized_train.txt','w')
		# if it's a tagged file, uncomment next line
		ENTITIES = open('tokenized_quality.txt','w')
		ENTITIES_POS = open('postags_quality.txt','w')
		#NEGATIVES = open('tokenized_negatives.txt') 
		with open(out_path,'w') as OUT:
			for content in self.cache:
				assert(len(content['tokens'])==len(content['pos']))
				for w in content['tokens'][:-1]:
					if len(w)==0:
						continue
					if w == '-LRB-':
						w = '('
					if w == '-RRB-':
						w = ')'
					if w.isupper() or w in self.punc:
						CASE.write('3')
					elif w[0].isupper():
						CASE.write('1')
					elif w.isdigit():
						CASE.write('4')
					else:
						CASE.write('0')
					OUT.write(self.inWordmapping(w.lower())+' ')
				OUT.write(content['tokens'][-1]+'\n')
				for p in content['pos']:
					POS_tag.write(p+'\n')
				for e in content['entityMentions']:
					tokens=[]
					#for t in content['tokens'][e['start']:e['end']]:
					#if u',' in content['tokens'][e[0]:e[1]]:
					#	continue
					for t in content['tokens'][e[0]:e[1]]:
						if t.lower() not in self.word_mapping:
							break
						else:
							tokens.append(str(self.word_mapping[t.lower()]))
					if len(tokens)==e[1]-e[0]:
					#if len(tokens)==e['end']-e['start']:
						ENTITIES.write(' '.join(tokens)+'\n')
						#ENTITIES_POS.write(' '.join(content['pos'][e['start']:e['end']])+'\n')
						ENTITIES_POS.write(' '.join(content['pos'][e[0]:e[1]])+'\n')
				#OUT.write('\n')
				CASE.write('\n')
		#ENTITIES.close()
		ENTITIES_POS.close()
		CASE.close()
		POS_tag.close()
	
	def tokenize_stopwords(self,in_path,out_path):
		OUT=open(out_path,'w')
		with open(in_path,'r') as IN:
			for line in IN:
				OUT.write(self.inTrainmapping(line.replace('\n','').lower())+'\n')
		OUT.close()

	def tokenize_phrases(self,input_file,outpath):
		#Phrases=open('tokenized_quality.txt','w')
		Phrases=open(outpath,'w')
		with open(input_file,'r') as IN:
			for line in IN:
				result=''
				for t in line.strip().split(' '):
					if t.lower() in self.word_mapping:
						result+=str(self.word_mapping[t.lower()])+' '
					else:
						result=''
						break
					#Phrases.write(str(self.word_mapping[t.lower()])+' ')
				if len(result)>0:
					Phrases.write(result+'\n')

	def dump(self):
		with open('token_mapping.txt','w') as OUT:
			for k,v in self.word_mapping.iteritems():
				OUT.write(str(v)+'\t'+k.encode('ascii', 'ignore').decode('ascii')+'\n')
		if self.mode=='Constraints Mode':
			with open('tokenized_punctuations.txt','w') as OUT:
				for k,v in self.punc_mapping.iteritems():
					OUT.write(str(k)+'\t'+v+'\n')
		cPickle.dump(self.test_token, open('test_token.p','wb'))
		cPickle.dump(self.test_word, open('test_word.p','wb'))
		cPickle.dump(self._punc, open('test_punc.p','wb'))

	def dump_raw(self,outpath):
		with open(outpath,'w') as OUT:
			for content in self.cache:
				OUT.write(' '.join(content['tokens'])+'\n')


	def load(self):
		self.test_token=cPickle.load(open('tmp_remine/test_token.p','rb'))
		self.test_word=cPickle.load(open('tmp_remine/test_word.p','rb'))

	def load_dict(self):
		with open('token_mapping.txt','r') as IN:
			for line in IN:
				tmp=line.strip().split('\t')
				if len(tmp)>=2:
					self.word_mapping[tmp[1]]=int(tmp[0])
			#print self.word_mapping
	
	def tokenize_test(self,test_path,out_path):
		CASE=open('case_'+out_path,'w')
		POS_tag=open('pos_tags_tokenized_text_to_seg.txt','w')
		OUT=open(out_path,'w')
		TEXT=open('raw_text_to_seg.txt','w')
		self.test_token=[]
		self.test_word=[]
		self._punc=[]
		sum_token=0
		with open(test_path,'r') as IN:
			for line in IN:
				content=json.loads(line)
				assert(len(content['tokens'])==len(content['pos']))
				token=[]
				word=[]
				punc_pos=[]
				for w in content['tokens'][:-1]:
					#if w=="``" or w=="''":
					#	continue
					if len(w)==0:
						continue
					if w == '-LRB-':
						w = '('
					if w == '-RRB-':
						w = ')'
					if w.isupper() or w in self.punc:
						CASE.write('3')
					elif w[0].isupper():
						CASE.write('1')
					elif w.isdigit():
						CASE.write('4')
					else:
						CASE.write('0')
					OUT.write(self.inTrainmapping(w.lower())+' ')
					if self.mode=='Constraints Mode' or w not in self.punc:
						token.append(self.inTrainmapping(w.lower()))
						word.append(w)
					else:
						punc_pos.append((len(word),w))
					punc_pos.append((len(word),content['tokens'][-1]))
					#if w != "``" and w!= "''": 
					#	TEXT.write(w+' ')
					#else:
					TEXT.write(w.encode('ascii', 'ignore').decode('ascii')+' ')
				for p in content['pos']:
					POS_tag.write(p+'\n')
				OUT.write('\n')
				CASE.write('\n')
				TEXT.write('\n')
				self.test_token.append(token)
				sum_token+=len(token)
				self.test_word.append(word)
				self._punc.append(punc_pos)
		print sum_token
		OUT.close()
		POS_tag.close()
		CASE.close()
		TEXT.close()



	def mapBack(self,seg_path,outpath):
		queue=[]
		r_ptr=0
		c_ptr=0
		start=['<None>','<ENTITY>','<RELATION>']
		end=['</None>','</ENTITY>','</RELATION>']
		dictx={'<ENTITY>':'[','<RELATION>':'(','</RELATION>':')','</ENTITY>':']'}
		with open(seg_path,'r') as _seg, open(outpath,'w') as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				in_phrase=False
				while (len(queue)>0):
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_token[r_ptr]):
							in_phrase=True
							if queue[0]!='<None>':
								OUT.write('\n'+dictx[queue.pop(0)])
							else:
								queue.pop(0)
								OUT.write('\n')
							r_ptr+=1
							c_ptr=0
							continue
						else:
							in_phrase=False
							if 'None' not in queue[0]:
								OUT.write(dictx[queue.pop(0)])
							else:
								queue.pop(0)
					elif c_ptr < len(self.test_token[r_ptr]) and queue[0] == self.test_token[r_ptr][c_ptr]:
						if not in_phrase or len(queue)>1 and queue[1] not in end:
							OUT.write(self.test_word[r_ptr][c_ptr].encode('ascii', 'ignore').decode('ascii')+' ')
						else:
							OUT.write(self.test_word[r_ptr][c_ptr].encode('ascii', 'ignore').decode('ascii'))
						queue.pop(0)
						c_ptr+=1
					else:
						#print 'here'
						r_ptr+=1
						c_ptr=0
						OUT.write('\n')

	def mapBackv2(self,seg_path,outpath):
		queue=[]
		r_ptr=0
		c_ptr=0
		start=['<None>','<ENTITY>','<RELATION>']
		end=['</None>','</ENTITY>','</RELATION>']
		start_phrase=False
		with open(seg_path,'r') as _seg, open(outpath,'w') as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				while (len(queue)>0):
					#print c_ptr,r_ptr
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_token[r_ptr]):
							#OUT.write('\n'+queue.pop(0)+' ')
							start_phrase=True
							OUT.write('\n')
							r_ptr+=1
							c_ptr=0
							queue.pop(0)
							continue
						else:
							if queue[0] in start:
								start_phrase=True
							else:
								start_phrase=False
								if 'None' in queue[0]:
									#OUT.write(':BP,')
									OUT.write(',')
								elif 'ENTITY' in queue[0]:
									OUT.write(':EP,')
								else:
									OUT.write(':RP,')
							queue.pop(0)
					elif c_ptr < len(self.test_token[r_ptr]) and queue[0] == self.test_token[r_ptr][c_ptr]:
						if start_phrase:
							OUT.write(self.test_word[r_ptr][c_ptr].encode('ascii', 'ignore').decode('ascii')+' ')
						else:
							OUT.write(self.test_word[r_ptr][c_ptr].encode('ascii', 'ignore').decode('ascii')+',')
						queue.pop(0)
						c_ptr+=1
					else:
						r_ptr+=1
						c_ptr=0
						OUT.write('\n')
				#break
	
	def mapBackv3(self,seg_path,outpath):
		queue=[]
		r_ptr=0
		c_ptr=0
		start=['<None>','<ENTITY>','<RELATION>']
		end=['</None>','</ENTITY>','</RELATION>']
		start_phrase=False
		with open(seg_path,'r') as _seg, codecs.open(outpath,'w',encoding="utf-8") as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				while (len(queue)>0):
					#print c_ptr,r_ptr
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_token[r_ptr]):
							#OUT.write('\n'+queue.pop(0)+' ')
							start_phrase=True
							OUT.write('\n')
							r_ptr+=1
							c_ptr=0
							queue.pop(0)
							continue
						else:
							if queue[0] in start:
								start_phrase=True
							else:
								start_phrase=False
								OUT.write(',')
							queue.pop(0)
					elif c_ptr < len(self.test_token[r_ptr]):
						if start_phrase:
							OUT.write(queue[0]+' ')
						else:
							OUT.write(queue[0]+',')
						queue.pop(0)
						c_ptr+=1
					else:
						#print 'here'
						r_ptr+=1
						c_ptr=0
						OUT.write('\n')		
		
if __name__ == '__main__':
	if sys.argv[1]=='translate':
		tmp=PreProcessor(sys.argv[2])
		tmp.mode = "Constraints Mode"
		tmp.tokenize_train(sys.argv[3])
		tmp.tokenize_test(sys.argv[4],sys.argv[5])
		tmp.tokenize_stopwords(sys.argv[6],sys.argv[7])
		tmp.tokenize_phrases(sys.argv[8], sys.argv[9])
		tmp.tokenize_phrases(sys.argv[10], sys.argv[11])
		tmp.dump()
	elif sys.argv[1]=='segmentation':
		tmp=PreProcessor(None)
		tmp.load()
		tmp.mapBackv2(sys.argv[2],sys.argv[3])
	elif sys.argv[1]=='raw':
		tmp=PreProcessor(sys.argv[2])
		tmp.dump_raw(sys.argv[3])
	elif sys.argv[1]=='temp':
		tmp=PreProcessor(None)
		tmp.load_dict()
		tmp.tokenize_phrases(sys.argv[2],sys.argv[3])
