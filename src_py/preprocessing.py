import sys
import argparse, _pickle
import nltk,json

class PreProcessor(object):
	"""docstring for PreProcessor"""
	def __init__(self):
		self.word_cnt = 1
		self.word_mapping = dict()
		self.punc_mapping = dict()
		self.punc = {'.',',','"',"'",'?',':',';','-','!','-lrb-','-rrb-','``',"''", ''}

	def case(self, w):
		if w.isupper() or w in self.punc:
			return '3'
		elif w[0].isupper():
			return '1'
		elif w.isdigit():
			return '4'
		else:
			return '0'

	def inWordmapping(self,word):
		if word not in self.word_mapping:
			self.word_mapping[word]=self.word_cnt
			if word in self.punc:
				self.punc_mapping[self.word_cnt] = word
			#self.reverse_mapping[self.word_cnt]=word
			self.word_cnt+=1
		return str(self.word_mapping[word])

	def chunk_train(self, docIn, posIn):

		grammar = r"""
			DATE:              # chunk sequences of proper nouns
      		{<NNP><CD>}
  			NP: {<DT|PP\$>?<JJ>*<NN|NNS>+}   # chunk determiner/possessive, adjectives and noun
      		{<NNP>+<IN><NNP>+}
      		{<NNP>+}
      		
		"""

		'''
		grammar = r"""
  			NP: {<DT|PP\$>?<JJ>*<NN>+}   # chunk determiner/possessive, adjectives and noun
      		{<NNP>+}                # chunk sequences of proper nouns
		"""
		'''
		cp = nltk.RegexpParser(grammar)
		cnt = 0
		with open(docIn, encoding='utf-8') as doc, open(posIn, encoding='utf-8') as pos, open('tmp_remine/boost_patterns.txt', 'w', encoding='utf-8') as out:
			for i,j in zip(doc, pos):
				d = i.strip().split(' ')
				p = j.strip().split(' ')
				assert(len(d) == len(p))
				sent = list(zip(d,p))
				tree = cp.parse(sent)
				for subtree in tree.subtrees():
					if subtree.label() == 'NP' or subtree.label() == 'DATE':
						branch = subtree.leaves()
						if len(branch) > 1:
							string = ' '.join(list(map(lambda x:x[0], branch)))
							if 'www' not in string and '.com' not in string:
								out.write(' '.join(list(map(lambda x:str(self.inWordmapping(x[0]))+'_'+x[1], branch))) + '\n')
								# out.write(' '.join(list(map(lambda x:x[0], branch))) + '\n')
					#if subtree.label() == 'DATE':
					#	branch = subtree.leaves()
					#	if len(branch) > 1:
							#print(' '.join(list(map(lambda x:x[0], branch))) + '\n')
							#chunks.add(map(lambda x:x[0]+'_'+x[1], branch))
			#print(len(chunks))

	def tokenized_test(self, docIn, posIn, depIn):
		fdep = open('tmp_remine/deps_test.txt', 'w')
		fpos = open('tmp_remine/pos_tags_test.txt', 'w')
		fdoc = open('tmp_remine/tokenized_test.txt', 'w')
		fcase = open('tmp_remine/case_tokenized_test.txt', 'w')

		self.test_tokens = []
		self.test_words = []
		with open(docIn, encoding='utf-8') as doc, open(posIn, encoding='utf-8') as pos, open(depIn, encoding='utf-8') as dep:
			for t,p,d in zip(doc,pos,dep):
				_word = []
				_token = []
				tokens = t.strip().split(' ')
				postags = p.strip().split(' ')
				deps = d.strip().split(' ')
				tmp = []
				for i,d in enumerate(deps):
					#dd=d.split('_')
					tmp.append(str(i)+'_'+d)
				assert(len(tokens) == len(postags) == len(deps))
				#print(tokens)
				for i,w in enumerate(tokens):
					if i != len(tokens) - 1:
						fdoc.write(self.inWordmapping(w)+' ')
						_token.append(self.inWordmapping(w))
					else:
						#if w not in self.punc:
						fdoc.write(self.inWordmapping(w)+'\n')
						_token.append(self.inWordmapping(w))
						#else:
							#fdoc.write(w+'\n')
							#_token.append(w)
					_word.append(w)
					fcase.write(self.case(w))
				self.test_tokens.append(_token)
				self.test_words.append(_word)
				fcase.write('\n')
				fdep.write('\n'.join(tmp) + '\n')
				fpos.write('\n'.join(postags) + '\n')

		fdep.close()
		fpos.close()
		fdoc.close()
		fcase.close()

	def tokenized_train(self, docIn, posIn, depIn):
		fdep = open('tmp_remine/deps_train.txt', 'w')
		fpos = open('tmp_remine/pos_tags_train.txt', 'w')
		fdoc = open('tmp_remine/tokenized_train.txt', 'w')
		fcase = open('tmp_remine/case_tokenized_train.txt', 'w')
		self.test_tokens = []
		self.test_words = []
		with open(docIn, encoding='utf-8') as doc, open(posIn, encoding='utf-8') as pos, open(depIn, encoding='utf-8') as dep:
			for t,p,d in zip(doc,pos,dep):
				_word = []
				_token = []
				tokens = t.strip().split(' ')
				postags = p.strip().split(' ')
				deps = d.strip().split(' ')
				tmp = []
				for i,d in enumerate(deps):
					dd=d.split('_')
					tmp.append(str(i)+'_'+dd[0])
				assert(len(tokens) == len(postags) == len(deps))
				for i,w in enumerate(tokens):
					if i != len(tokens) - 1:
						fdoc.write(self.inWordmapping(w)+' ')
						_token.append(self.inWordmapping(w))
					else:
						if w not in self.punc:
							fdoc.write(self.inWordmapping(w)+'\n')
							_token.append(self.inWordmapping(w))
						else:
							fdoc.write(w+'\n')
					_word.append(w)
					fcase.write(self.case(w))
				self.test_tokens.append(_token)
				self.test_words.append(_word)
				fcase.write('\n')
				fdep.write('\n'.join(tmp) + '\n')
				fpos.write('\n'.join(postags) + '\n')
		fdep.close()
		fpos.close()
		fdoc.close()
		fcase.close()

	def tokenized_train_rm(self, docIn):
		fpos = open('tmp_remine/rm_pos_tags_train.txt', 'w')
		fdoc = open('tmp_remine/rm_tokenized_train.txt', 'w')
		fcase = open('tmp_remine/rm_case_tokenized_train.txt', 'w')
		self.test_tokens = []
		self.test_words = []
		with open(docIn) as IN:
			for line in IN:
				_word = []
				_token = []
				tmp = json.loads(line)
				tokens = tmp['tokens']
				postags = tmp['pos']
				for i,w in enumerate(tokens):
					if i != len(tokens) - 1: 
						fdoc.write(str(self.word_mapping[w])+' ')
					else:
						fdoc.write(str(self.word_mapping[w])+'\n')
					_token.append(str(self.word_mapping[w]))
					_word.append(w)
					fcase.write(self.case(w))
				self.test_tokens.append(_token)
				self.test_words.append(_word)
				fcase.write('\n')
				fpos.write('\n'.join(postags) + '\n')
		fpos.close()
		fdoc.close()
		fcase.close()


	def dump_rm(self):
		_pickle.dump(self.test_words, open('tmp_remine/rm_test_words.p', 'wb'))
		_pickle.dump(self.test_tokens, open('tmp_remine/rm_test_tokens.p', 'wb'))

	def dump_test(self):
		_pickle.dump(self.test_words, open('tmp_remine/real_test_words.p', 'wb'))
		_pickle.dump(self.test_tokens, open('tmp_remine/real_test_tokens.p', 'wb'))

	def dump(self):
		with open('tmp_remine/tokenized_punctuations.txt','w', encoding='utf-8') as OUT:
			for k,v in self.punc_mapping.items():
				OUT.write(str(k)+'\t'+v+'\n')
		with open('tmp_remine/token_mapping.txt','w', encoding='utf-8') as OUT:
			for k,v in self.word_mapping.items():
				OUT.write(str(v)+'\t'+k+'\n')
		_pickle.dump(self.word_mapping, open('tmp_remine/token_mapping.p', 'wb'))
		_pickle.dump(self.test_words, open('tmp_remine/test_words.p', 'wb'))
		_pickle.dump(self.test_tokens, open('tmp_remine/test_tokens.p', 'wb'))

	def load(self):
		self.word_mapping = _pickle.load(open('tmp_remine/token_mapping.p', 'rb'))

	def load_all(self):
		self.word_mapping = _pickle.load(open('tmp_remine/token_mapping.p', 'rb'))
		self.test_tokens = _pickle.load(open('tmp_remine/test_tokens.p', 'rb'))
		self.test_words = _pickle.load(open('tmp_remine/test_words.p', 'rb'))

	def load_test(self):
		self.word_mapping = _pickle.load(open('tmp_remine/token_mapping.p', 'rb'))
		self.test_tokens = _pickle.load(open('tmp_remine/real_test_tokens.p', 'rb'))
		self.test_words = _pickle.load(open('tmp_remine/real_test_words.p', 'rb'))

	def tokenize(self, docIn, docOut):
		with open(docIn, encoding='utf-8') as doc, open(docOut,'w', encoding='utf-8') as out:
			for line in doc:
				tmp = line.strip().split(' ')
				out.write(' '.join(list(map(lambda x: self.inWordmapping(x), tmp)))+'\n')

	def mapBack(self,seg_path,outpath):
		queue=[]
		r_ptr=0
		c_ptr=0
		#start=['<None>','<ENTITY>','<RELATION>']
		#end=['</None>','</ENTITY>','</RELATION>']

		start=['<None>','<EP>','<RP>','<BP>']
		end=['</None>','</EP>','</RP>', '</BP>']
		dictx={'<EP>':'[ ','<RP>':'<','</EP>':'] ','</RP>':'>', '<BP>':'(', '</BP>':')'}
		with open(seg_path,'r', encoding='utf-8') as _seg, open(outpath,'w', encoding='utf-8') as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				in_phrase=False
				while (len(queue)>0):
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_tokens[r_ptr]):
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
					elif c_ptr < len(self.test_tokens[r_ptr]) and queue[0] == self.test_tokens[r_ptr][c_ptr]:
						if not in_phrase or len(queue)>1 and queue[1] not in end:
							OUT.write(self.test_words[r_ptr][c_ptr]+' ')
						else:
							OUT.write(self.test_words[r_ptr][c_ptr])
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
		start=['<None>','<EP>','<RP>','<BP>']
		end=['</None>','</EP>','</RP>', '</BP>']
		start_phrase=False
		with open(seg_path,'r', encoding='utf-8') as _seg, open(outpath,'w', encoding='utf-8') as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				while (len(queue)>0):
					#print c_ptr,r_ptr
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_tokens[r_ptr]):
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
									OUT.write(']_[')
								elif 'EP' in queue[0]:
									OUT.write(':EP]_[')
								elif 'RP' in queue[0]:
									OUT.write(':RP]_[')
								elif 'BP' in queue[0]:
									OUT.write(':BP]_[')
							queue.pop(0)
					elif c_ptr < len(self.test_tokens[r_ptr]) and queue[0] == self.test_tokens[r_ptr][c_ptr]:
						if start_phrase:
							OUT.write(self.test_words[r_ptr][c_ptr] + ' ')
						else:
							OUT.write(self.test_words[r_ptr][c_ptr] + ']_[')
						queue.pop(0)
						c_ptr+=1
					else:
						r_ptr+=1
						c_ptr=0
						OUT.write('\n')

	def map(self,seg_path,outpath):
		queue=[]
		r_ptr=0
		c_ptr=0
		start=['<None>','<EP>','<RP>','<BP>']
		end=['</None>','</EP>','</RP>', '</BP>']
		start_phrase=False
		with open(seg_path,'r', encoding='utf-8') as _seg, open(outpath,'w', encoding='utf-8') as OUT:
			for line in _seg:
				for token in line.strip().split(' '):
					queue.append(token)
				#print queue
				while (len(queue)>0):
					#print c_ptr,r_ptr
					if queue[0] in start or queue[0] in end:
						#if queue[0] == '</phrase>' or c_ptr < len(self.test_token[r_ptr]):
						if queue[0] in start and c_ptr == len(self.test_tokens[r_ptr]):
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
									OUT.write(']_[')
								elif 'EP' in queue[0]:
									OUT.write(':EP]_[')
								elif 'RP' in queue[0]:
									OUT.write(':RP]_[')
								elif 'BP' in queue[0]:
									OUT.write(':BP]_[')
							queue.pop(0)
					elif c_ptr < len(self.test_tokens[r_ptr]) and queue[0] == self.test_tokens[r_ptr][c_ptr]:
						if start_phrase:
							OUT.write(self.test_words[r_ptr][c_ptr] + ' ')
						else:
							OUT.write(self.test_words[r_ptr][c_ptr] + ']_[')
						queue.pop(0)
						c_ptr+=1
					else:
						r_ptr+=1
						c_ptr=0
						OUT.write('\n')


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description="Run node2vec.")
	parser.add_argument('--in1', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')
	parser.add_argument('--in2', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')
	parser.add_argument('--in3', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')

	parser.add_argument('--out', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')

	parser.add_argument('--op', help='Type of supervision')

	args = parser.parse_args()

	tmp = PreProcessor()
	if args.op == 'train':
		tmp.tokenized_train(args.in1, args.in2, args.in3)
		tmp.dump()
	elif args.op == 'test':
		tmp.load()
		tmp.tokenized_test(args.in1, args.in2, args.in3)
		tmp.dump_test()
	elif args.op == 'chunk':
		tmp.load()
		tmp.chunk_train(args.in1, args.in2)
	elif args.op == 'translate':
		tmp.load()
		tmp.tokenize(args.in1, args.out)
	elif args.op == 'segment':
		tmp.load_all()
		tmp.mapBackv2(args.in1, args.out)
	elif args.op == 'train_rm':
		tmp.load()
		tmp.tokenized_train_rm(args.in1)
		tmp.dump_rm()
	elif args.op == 'segment_test':
		tmp.load_test()
		tmp.mapBackv2(args.in1, args.out)

