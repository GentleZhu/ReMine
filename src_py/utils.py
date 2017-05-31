import json
import sys
from collections import defaultdict
import pickle
import operator
def cvtRaw(file_path,out_path,num):
	OUT=open(out_path,'w')
	for i in xrange(num):
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

def eliminateTab(file_path,out_path):
	with open(file_path) as IN, open(out_path,'w') as OUT:
		for line in IN:
			OUT.write(line.split('\t')[1].lstrip())
def relationLinker(postag_path,file_path):
	matched_phrases=defaultdict(int)
	relation_pattern=set()
	references=[]
	with open(postag_path) as IN:
		for line in IN:
			if ' ' in line:
				relation_pattern.add(line.strip())
	for i in list(relation_pattern):
		references.append(i.split(' '))

	with open(file_path,'r') as IN:
		cnt=0
		for line in IN:
			if cnt%1000==0:
				print cnt
			cnt+=1
			#print line
			tmp=json.loads(line)
			for i,e in enumerate(tmp['entityMentions']):
				if i == len(tmp['entityMentions'])-1:
					break
				#print tmp['entityMentions'][i+1][0]-e[1]
				for r in references:
					length=len(r)
					if tmp['entityMentions'][i+1][0]-e[1] >= length:
						for idx in xrange(e[1],tmp['entityMentions'][i+1][0]):
							if tmp['pos'][idx:idx+length] == r:
								#print r,tmp['tokens'][idx:idx+length]
								matched_phrases[' '.join(tmp['tokens'][idx:idx+length])]+=1
			#break
	pickle.dump(matched_phrases,open('dumped_relations.p','wb'))

def playRelations():
	matched_phrases=pickle.load(open('dumped_relations.p','rb'))
	for k,v in matched_phrases.iteritems():
		if v>20:
			print k.encode('ascii', 'ignore').decode('ascii')
def cvtTaggedRaw(file_path,out_path):
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		for line in IN:
			corpus=dict()
			corpus['tokens']=[]
			corpus['pos']=[]
			for e in line.strip().split(' '):
				corpus['tokens'].append('_'.join(e.split('_')[:-1]))
				corpus['pos'].append(e.split('_')[-1])
			corpus['sentText']=' '.join(corpus['tokens'])
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
		for k,v in entities.iteritems():
			if v>=5 and ' ' in k:
				freq_entities[k.count(' ')+1].add(k)
			elif v>30 and v<100:
				freq_entities[k.count(' ')+1].add(k)
		print freq_entities.keys()
		print len(freq_entities[1])
		pickle.dump(freq_entities, open(out_path,'wb'))

		#OUT.write(json.dumps(entities))
	#print sum_segs

def entityLinker(file_path,seed_path,out_path=None):
	seeds=defaultdict(list)
	with open(seed_path,'r') as seed:
		for line in seed:
			tmp=line.strip().split('\t')
			seeds[int(tmp[0])].append((tmp[1].split(' '),tmp[3]))
			#break
		#print seeds
	#print seeds[0]
	#return
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		cnt=0
		sum_match=0
		for line in IN:
			ptr=0
			tmp=json.loads(line)
			tmp['entityMentions']=[]
			for e in seeds[cnt]:
				window_size=len(e[0])
				restore_ptr=ptr
				found=False
				while tmp['tokens'][ptr:ptr+window_size]!=e[0] and ptr<len(tmp['tokens']):
					ptr+=1
					found=True
				if ptr>=len(tmp['tokens']):
					ptr=restore_ptr
				elif found:
					tmp['entityMentions'].append([ptr,ptr+window_size,e[1]])
					sum_match+=1
			
			OUT.write(json.dumps(tmp)+'\n')
			cnt+=1
			#break
		#for a,b in zip(IN,seed):
		print sum_match

def refinePos(file_path,out_path):
	patterns=defaultdict(int)
	with open(file_path,'r') as IN:
		for line in IN:
			#if ' ' in line:
			if ' ' in line.strip():
				patterns[line.strip()]+=1
	sorted_x = sorted(patterns.items(), key=operator.itemgetter(1),reverse=True)
	
	with open(out_path,'w') as OUT:
		for x in sorted_x[:150]:
			OUT.write(x[0]+'\n')



def addSentID(file_path,seed_path):
	seeds=[]
	with open(seed_path,'r') as seed:
		for line in seed:
			tmp=set(line.strip().split(' '))
			seeds.append(tmp)
	print len(seeds)
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
			print cnt,ptr


def entityLinker2(file_path,seed_path,out_path,len_min=1,len_max=6):
	seeds=defaultdict(list)
	print len_min
	seeds=pickle.load(open(seed_path,'rb'))
	#print seeds
	with open(file_path,'r') as IN, open(out_path,'w') as OUT:
		
		cnt=0
		
		for line in IN:
			tmp=json.loads(line)
			assert(len(tmp['tokens'])==len(tmp['pos']))
			tmp['entityMentions']=[]
			for i in xrange(len_min,len_max):
				ptr=0
				while ptr+i <= len(tmp['tokens']):
					temp=' '.join(tmp['tokens'][ptr:ptr+i])
					if temp in seeds[i]:
						tmp['entityMentions'].append([ptr,ptr+i,temp])
					ptr+=1
			tmp['entityMentions']=sorted(tmp['entityMentions'])
			OUT.write(json.dumps(tmp)+'\n')
			if cnt%1000 ==0:
				print cnt
			cnt+=1

def cvtTest(file_path,out_path=None):
	test_json=[]
	with open(file_path) as IN,open(out_path,'w') as OUT:
		tmp=json.load(IN)
		print len(tmp['documents'])
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

if __name__ == '__main__':
	#cvtRaw(sys.argv[1],sys.argv[2],int(sys.argv[3]))
	#entityLinker(sys.argv[1],sys.argv[2],sys.argv[3])
	#cvtTest(sys.argv[1],sys.argv[2])
	#relationLinker(sys.argv[1],sys.argv[2])
	#playRelations()
	#cvtTaggedRaw(sys.argv[1],sys.argv[2])
	#cntSegs(sys.argv[1])
	#cvtUntaggedRaw(sys.argv[1],sys.argv[2],sys.argv[3])
	#entityLinker2(sys.argv[1],sys.argv[2],sys.argv[3])
	refinePos(sys.argv[1],sys.argv[2])
	#addSentID(sys.argv[1],sys.argv[2])
