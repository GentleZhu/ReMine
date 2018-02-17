import json
import sys
from collections import defaultdict
import _pickle as cPickle
#import cPickle
import operator
import re

def cvtRaw(file_path,out_path,num):
	OUT=open(out_path,'w')
	for i in range(num):
		with open(file_path+str(i)+'.txt.json','r') as IN:
			tmp=json.load(IN)
			sentence=dict()
			sentence['tokens']=[]
			sentence['pos']=[]
			sentence['ner']=[]
			for tt in tmp['sentences']:
				for t in tt['tokens']:
					sentence['tokens'].append(t['word'])
					sentence['pos'].append(t['pos'])
					#sentence['ner'].append(t['ner'])
			OUT.write(json.dumps(sentence)+'\n')
		#break
	OUT.close()

def getEntity(file_path, output, isPos=False):
	entities = defaultdict(int)
	with open(file_path) as IN, open(output, 'w') as OUT:
		for line in IN:
			tmp=json.loads(line)
			for e in tmp['entityMentions']:
				if isPos:
					key = ' '.join(tmp['pos'][e[0]:e[1]])
				else:
					key = e[2]
				#if 'V' in key:
				#	print e[2],key
				entities[key]+=1
		for k,v in entities.items():
			if v > 9:
				OUT.write(k+'\n')
				#if e[1] - e[0] == 1:
				#	print ' '.join(tmp['tokens'][e[0]:e[1]])

def eliminateTab(file_path,out_path):
	with open(file_path) as IN, open(out_path,'w') as OUT:
		for line in IN:
			OUT.write(line.split('\t')[1].lstrip())
def relationLinker(file_path, prefix=""):
	relation_token=set(["VB","VBD","VBG","VBN","VBP","VBZ"])
	V_pattern = "((VB|VBD|VBG|VBN|VBN|VBP|VBZ) )+"
	#P_pattern = "((IN|RP) ?)"
	P_pattern = "((IN|RP) +)"
	W_pattern = "((NN.{0,2}|JJ.{0,1}|RB.{0,1}|PRP.{0,1}|DT ))+"
	N_pattern = "(NN.{0,2})+"
	relation_pattern = V_pattern+W_pattern+P_pattern+"|"+V_pattern+P_pattern+"|"+V_pattern+"|"+N_pattern+P_pattern
	print(relation_pattern)
	matched_phrases=defaultdict(int)
	matched_unigram=defaultdict(int)
	#relation_pattern=set()
	#references=[]
	'''
	with open(postag_path) as IN:
		for line in IN:
			if ' ' in line:
				relation_pattern.add(line.strip())
	for i in list(relation_pattern):
		references.append(i.split(' '))
	'''

	with open(file_path,'r') as IN:
		cnt=0
		for line in IN:
			#if cnt > 100:
			#	break
			cnt+=1
			#print(cnt)
			#print line
			tmp=json.loads(line)

			for i,e in enumerate(tmp['entityMentions']):
				for j in range(i+1, len(tmp['entityMentions'])):
					if tmp['entityMentions'][j][0]-e[1] >= 0:
						candidate = ' '.join(tmp['pos'][e[1]:tmp['entityMentions'][j][0]])
						m = re.search(relation_pattern, candidate)
						if m:
							result=m.group(0).rstrip()
							length = result.count(' ')
							#print length
							index=candidate.find(m.group(0))
							#print candidate,index
							index = candidate[:index].count(' ')
							#print candidate
							#print index,m.group(0)
							relations=' '.join(tmp['tokens'][e[1]:tmp['entityMentions'][j][0]][index:index+length+1])
							matched_phrases[relations]+=1
							matched_unigram[result]+=1
					#if tmp['entity_mentions'][i+1]['start']-e['end'] >= length:
					'''
						for idx in range(e[1],tmp['entity_mentions'][i+1][0]):
							if tmp['pos'][idx:idx+length] == r:
								#print r,tmp['tokens'][idx:idx+length]
								matched_phrases[' '.join(tmp['tokens'][idx:idx+length])]+=1
								#print ' '.join(tmp['tokens'][idx:idx+length])
							if tmp['pos'][idx] in relation_token:
								matched_unigram[tmp['tokens'][idx]]+=1
					'''

	cPickle.dump(matched_phrases,open('tmp/' + prefix + 'dumped_relations.p','wb'))
	cPickle.dump(matched_unigram,open('tmp/' + prefix + 'dumped_relations_pattern.p','wb'))

def dumpRelations(prefix, output, isPos=False):
	if isPos:
		matched_phrases=cPickle.load(open('tmp/' + prefix + 'dumped_relations_pattern.p','rb'))
	else:
		matched_phrases=cPickle.load(open('tmp/' + prefix + 'dumped_relations.p','rb'))
	
	with open(output, 'w', encoding='utf-8') as OUT:
		for k,v in matched_phrases.items():
			if v > 9:
				OUT.write(k + '\n')

def cvtTaggedRaw(file_path,out_path):
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		for line in IN:
			corpus=dict()
			corpus['tokens']=[]
			corpus['pos']=[]
			for e in line.strip().split(' '):
				corpus['tokens'].append('_'.join(e.split('_')[:-1]))
				corpus['pos'].append(e.split('_')[-1])
			#corpus['sentText']=' '.join(corpus['tokens'])
			OUT.write(json.dumps(corpus)+'\n')

def cvtUntaggedRaw(file_path,seed_path,out_path):
	seeds=defaultdict(list)
	with open(seed_path,'r') as seed:
		for line in seed:
			tmp=line.strip().split('\t')
			seeds[int(tmp[0])].append(tmp[1])

	entities=defaultdict(int)
	with open(file_path,'r') as IN:
		cnt=0
		for line in IN:
			for e in seeds[cnt]:
				if e in line:
					entities[e]+=1
			cnt+=1
			if cnt>len(seeds):
				break
		freq_entities=defaultdict(set)
		for k,v in entities.items():
			if v>=5 and ' ' in k:
				freq_entities[k.count(' ')+1].add(k)
			elif v>30 and v<100:
				freq_entities[k.count(' ')+1].add(k)
		print(freq_entities.keys())
		print(len(freq_entities[1]))
		cPickle.dump(freq_entities, open(out_path,'wb'))

		#OUT.write(json.dumps(entities))
	#print sum_segs

def entityLinker(file_path,seed_path,out_path=None):
	seeds=defaultdict(list)
	with open(seed_path,'r') as seed:
		for line in seed:
			tmp=line.strip().split('\t')
			if float(tmp[4]) > 0.8 and float(tmp[5]) < 0.001:
				seeds[int(tmp[0])].append(tmp[1])
			#break
		#print seeds
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		cnt=0
		sum_match=0
		for line in IN:
			
			tmp=json.loads(line)
			tmp['entityMentions']=[]
			for e in seeds[cnt]:
				window_size=e.count(' ') + 1
				#if window_size == 1:
				#	continue
				
				found=False
				ptr = 0
				while ptr+window_size <= len(tmp['tokens']):
					ptr+=1
					if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == e:
						found=True
						break

				if found:
					tmp['entityMentions'].append([ptr,ptr+window_size,e])
					sum_match+=1
			tmp['entityMentions'].sort(key=operator.itemgetter(1))
			OUT.write(json.dumps(tmp)+'\n')
			cnt+=1
			#break
		#for a,b in zip(IN,seed):
		print(sum_match)

def refinePos(file_path,out_path):
	patterns=defaultdict(int)
	with open(file_path,'r') as IN:
		for line in IN:
			#if ' ' in line:
			if ' ' in line.strip():
				patterns[line.strip()]+=1
	sorted_x = sorted(patterns.items(), key=operator.itemgetter(1),reverse=True)
	
	with open(out_path,'w') as OUT:
		for x in sorted_x[:200]:
			if 'V' not in x[0] and 'N' in x[0]:
				OUT.write(x[0]+'\n')

def flatData(file_path,out_path):
	punc=['.',',','"',"'",'?',':',';','-','!','(',')','``',"''"]
	with open(file_path) as IN, open(out_path,'w') as OUT:
		for line in IN:
			temp=json.loads(line)['pos']
			#OUT.write(' '.join(json.loads(line)['pos']).encode('ascii', 'ignore').decode('ascii')+'\n')
			OUT.write(' '.join(temp).encode('ascii', 'ignore').decode('ascii')+'\n')
			#OUT.write(line.split('\t')[1])


def segment_combine(segment_path,out_path):
	with open(segment_path) as IN, open(out_path,'w') as OUT:
		for line in IN:
			for t in line.split(','):
				OUT.write('_'.join(t.rstrip().split(' '))+' ')
			OUT.write('\n')

def cvtTow2v(in_path,out_path):
	with open(in_path,'r') as IN, open(out_path,'w') as OUT:
		for line in IN:
			for phrase in line.replace(':EP','').replace(':RP','').split(','):
				OUT.write(phrase.strip().replace(' ','_').lower()+' ')


def addSentID(file_path,seed_path):
	seeds=[]
	with open(seed_path,'r') as seed:
		for line in seed:
			tmp=set(line.strip().split(' '))
			seeds.append(tmp)
	print(len(seeds))
	#return
	with open(file_path,'r') as IN:
		ptr=0
		cnt=0
		for line in IN:
			tmp=set(json.loads(line)['tokens'])
			if ptr == len(seeds) -1:
				break
			elif len(tmp&seeds[ptr+1])>0.6*len(tmp) or len(tmp&seeds[ptr+1])>len(tmp&seeds[ptr]):
				ptr+=1
			elif len(tmp&seeds[ptr])>0:
				#print ptr
				pass
			
			cnt+=1
			print(cnt,ptr)


def entityLinker2(file_path,seed_path,out_path,len_min=1,len_max=6):
	seeds=defaultdict(list)
	print(len_min)
	seeds=cPickle.load(open(seed_path,'rb'))
	#print seeds
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		
		cnt=0
		
		for line in IN:
			tmp=json.loads(line)
			assert(len(tmp['tokens'])==len(tmp['pos']))
			tmp['entityMentions']=[]
			for i in range(len_min,len_max):
				ptr=0
				while ptr+i <= len(tmp['tokens']):
					temp=' '.join(tmp['tokens'][ptr:ptr+i])
					if temp in seeds[i]:
						tmp['entityMentions'].append([ptr,ptr+i,temp])
					ptr+=1
			tmp['entityMentions']=sorted(tmp['entityMentions'])
			OUT.write(json.dumps(tmp)+'\n')
			if cnt%1000 ==0:
				print(cnt)
			cnt+=1

def cvtTest(file_path,out_path=None):
	test_json=[]
	with open(file_path) as IN,open(out_path,'w') as OUT:
		tmp=json.load(IN)
		print(len(tmp['documents']))
		for d in tmp['documents']:
			for s in d['sentences']:
				assert(len(s['pos'])==len(s['tokens']))
				if len(s['gold'])>0:
					entityMentions=[]
					for e in s['gold']:
						#print e['start'],e['end']
						#print ' '.join(s['tokens'][e['start']:e['end']])
						#print e['token']
						assert(' '.join(s['tokens'][e['start']-1:e['end']])==e['token'])
						entityMentions.append([e['start'],e['end'],e['label']])

					test_json.append({'tokens':s['tokens'],'pos':s['pos'],'entityMentions':entityMentions})
		
			#break
		for line in test_json:
			OUT.write(json.dumps(line)+'\n')
		#print len(tmp['documents'][0]['sentences'])

def removeDups(file_path, output):
	with open(file_path) as IN, open(output, 'w') as OUT:
		for line in IN:
			tmp = json.loads(line)
			ems = ''
			for em in tmp['entityMentions']:
				if len(em[2]) > 0:
					ems += str(em[0]) + '_'  + str(em[1]) + ' '
			OUT.write(ems.strip()+'\n')

def convertTest(file_path, output):
	with open(file_path) as IN, open(output, 'w') as OUT:
		for line in IN:
			tmp = json.loads(line)
			OUT.write(tmp['sentText'])

def addIndex(file_path, output):
	with open(file_path) as IN, open(output, 'w') as OUT:
		for line in IN:
			tmp = line.strip().split(' ')
			for idx,i in enumerate(tmp):
				tmp[idx] = str(idx) + '_' + i
			OUT.write(' '.join(tmp) + '\n')

def replaceLemma(in1, in2, out):
	wrong_lemma = set(['he', 'she', 'you'])
	with open(in1) as a, open(in2) as b, open(out, 'w') as c:
		for line_a, line_b in zip(a,b):
			tmp_a = line_a.strip().split(' ')
			tmp_b = line_b.strip().split(' ')
			tmp_c = []
			assert(len(tmp_a) == len(tmp_b))
			for w_a,w_b in zip(tmp_a,tmp_b):
				if w_a in wrong_lemma:
					tmp_c.append(w_b)
				else:
					tmp_c.append(w_a)
			c.write(' '.join(tmp_c) + '\n')

def compare(in1, in2):
	with open(in1) as a, open(in2) as b:
		sum_a = 0
		sum_b = 0
		sum_correct = 0
		#stopwords = ['the', 'and', 'of', 'at', 'in']
		stopwords = []
		for line_a,line_b in zip(a,b):
			tmp_a = json.loads(line_a)['entityMentions']
			tmp_b = json.loads(line_b)['entityMentions']
			pred_a = set(map(lambda x: x['text'], tmp_a))
			pred_b = set()
			for item in tmp_b:
				'''
				ttt = item[2].strip().split(' ')
				for idx, d in enumerate(ttt):
					if d in stopwords:
						del ttt[idx]
				'''
				#if item[2] != 'Mr.' and item[2] != 'Mrs.' and item[2] != 'Ms.':
				#	pred_b.add(item[2])
				#if len(item[2].split(' ')):
				pred_b.add(item[2])
			print(pred_a, pred_b)
			sum_a += len(pred_a)
			sum_b += len(pred_b)
			sum_correct += len(pred_a&pred_b)
		print(sum_a,sum_b,sum_correct)


def abalations(in1, in2):
	cnt = 0
	_sum = 0
	with open(in1) as a, open(in2) as b:
		for line_a,line_b in zip(a,b):
			scores = line_a.strip().split('\t')
			if len(scores[3].split('_')) > 4:
				scores[1] = str(float(scores[1])+3)
				print('\t'.join(scores))
				_sum += 1
				cnt += int(line_b.strip())
			else:
				print(line_a.strip())
	#print(float(cnt)/_sum)



if __name__ == '__main__':
	#TODO(branzhu): add comments for following methods

	#cvtRaw(sys.argv[1],sys.argv[2],int(sys.argv[3]))
	#entityLinker(sys.argv[1],sys.argv[2],sys.argv[3])
	#cvtTest(sys.argv[1],sys.argv[2])
	#relationLinker(sys.argv[1],sys.argv[2],sys.argv[3])
	#playRelations(sys.argv[1])
	#getEntity(sys.argv[1])
	
	#cvtTaggedRaw(sys.argv[1],sys.argv[2])
	#cntSegs(sys.argv[1])
	#cvtUntaggedRaw(sys.argv[1],sys.argv[2],sys.argv[3])
	
	#entityLinker(sys.argv[1], sys.argv[2], sys.argv[3])
	#relationLinker(sys.argv[1], sys.argv[2])

	#playRelations(sys.argv[1])
	#removeDups(sys.argv[1], sys.argv[2])
	#convertTest(sys.argv[1], sys.argv[2])
	addIndex(sys.argv[1], sys.argv[2])
	
	#replaceLemma(sys.argv[1], sys.argv[2], sys.argv[3])
	#compare(sys.argv[1], sys.argv[2])
	
	#abalations(sys.argv[1], sys.argv[2])
	
	#flatData(sys.argv[1], sys.argv[2])

	#entityLinker2(sys.argv[1],sys.argv[2],sys.argv[3])
	#refinePos(sys.argv[1],sys.argv[2])
	#segment_combine(sys.argv[1],sys.argv[2])
	#flatData(sys.argv[1],sys.argv[2])
	#addSentID(sys.argv[1],sys.argv[2])

	#results_remine/segmentation.tokens utils/word2vec/data
	#cvtTow2v(sys.argv[1],sys.argv[2])
