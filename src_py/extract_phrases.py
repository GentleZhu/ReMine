import sys
if __name__ == '__main__':
	with open(sys.argv[1]) as IN, open(sys.argv[2],'w') as OUT, open(sys.argv[2]+'.type','w') as OUTY:
		for line in IN:
			for p in line.strip().split(','):
				if 'EP' in p:
					OUT.write(p.rstrip(' :EP')+'\t')
					OUTY.write('ENTITY'+'\t')
				elif 'RP' in p:
					OUT.write(p.rstrip(' :RP')+'\t')
					OUTY.write('RELATION'+'\t')
			OUT.write('\n')
			OUTY.write('\n')