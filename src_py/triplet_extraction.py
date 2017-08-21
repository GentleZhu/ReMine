import numpy as np

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
	tmp=Extractor({'emb_path':'/shared/data/qiz3/ReMine/utils/word2vec/data/demo.txt.bin',
		'corpus_path':'/shared/data/qiz3/ReMine/results_remine/sample.txt'})
	tmp.load_emb()
	print "emb loaded"
	#print tmp.emb['born']
	tmp.load_corpus()