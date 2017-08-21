import re
import sys
pattern=re.compile("(\w+?)<\/(.+?)>|<.+?>(.*?)<\/(.+?)>")
IN=open(sys.argv[1],'r')
cnt=0
with open(sys.argv[2],'w') as OUT:
	OUTY=open(sys.argv[2]+'.type','w')
	for line in IN:
		match=re.findall(pattern,line)
		if match:
			#print match
			for m in match:
				if m[0]:
					OUT.write(m[0]+'\t')
					OUTY.write(m[1]+'\t')
				else:
					OUT.write(m[2]+'\t')
					OUTY.write(m[3]+'\t')
		OUT.write('\n')
		OUTY.write('\n')
IN.close()
