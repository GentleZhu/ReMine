import json
import sys

if __name__ == '__main__':
	with open(sys.argv[1],'r') as IN,open(sys.argv[2],'w') as OUT:
		for line in IN:
			tmp=json.loads(line.strip())
			annotation=['O']*len(tmp['tokens'])
			for e in tmp['entityMentions']:
				annotation[e[0]:e[1]]=[e[2]]*(e[1]-e[0])
			
			for i,j in zip(tmp['tokens'],annotation):
				OUT.write(i+'\t'+j+'\n')
		
