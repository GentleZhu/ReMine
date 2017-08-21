import json
import codecs
import sys

def subfinder(mylist, pattern):
    #matches = []
    for i in range(len(mylist)):
        if mylist[i] == pattern[0] and mylist[i:i+len(pattern)] == pattern:
		return i
            #matches.append(i)   
    return None

if __name__=="__main__":
	with codecs.open(sys.argv[1],'r','utf-8') as INX:
		contentX=INX.readlines()
	with open(sys.argv[2]) as INY:
		contentY=INY.readlines()
	with open(sys.argv[3]) as INZ:
		contentZ=INZ.readlines()
	#print len(contentX)
	#print len(contentY)
	#print len(contentZ)
	OUT=open(sys.argv[4],'w')
	for x,y,z in zip(contentX,contentY,contentZ):
		tmp=json.loads(x)
		tmp['entity_mentions']=[]
		for i,j in zip(y.strip().split('\t'),z.strip().split('\t')):
			if len(i)==0:
				continue
			pattern=i.split(' ')
			index=subfinder(tmp['tokens'],pattern)
			if index is None and len(pattern)>0:
				#print tmp['tokens'],pattern
				continue
			tmp['entity_mentions'].append([index, index+len(pattern),j])
		OUT.write(json.dumps(tmp)+'\n')
	OUT.close()
	#print subfinder([1,2,3,1,2,3,4],[1,2,3,4])
