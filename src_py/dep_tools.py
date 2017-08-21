#python src_py/dep_tools.py ../dependency_parsing/stanford-parser-full-2017-06-09/data/english-onesent.txt.out
#performance 60K sentences - one thread, 16s
import re
import pickle
import sys
from collections import defaultdict
import json

class dep_tool(object):
	"""docstring for dep_tool"""
	def __init__(self, arg):
		self.arg = arg
		self.reg_pattern = "([a-z]+|.+)\(([a-z]+|.+)\-(\d+), (.+)\-(\d+)\)"

	def check_deps(self):
		with open(self.arg['dep_file'], 'r') as IN, open(self.arg['output'],'r') as REF:
			line_num = 0
			for line,json_line in zip(IN,REF):
				tmp=line.strip().split(' ')
				j_tmp=json.loads(json_line)

				if (len(tmp) != len(j_tmp['tokens'])):
					print len(j_tmp['tokens'])
					print len(tmp)
					print j_tmp['tokens']
					print tmp
					break
				line_num += 1
			print 


	def read_deps(self):
		with open(self.arg['dep_file'], 'r') as IN, open(self.arg['dump_file'], 'w') as OUT:
		#,open(self.arg['output'],'r') as REF:
			dep=[]
			line_num = 0
			#for line,json_line in zip(IN,REF):
			for line in IN:
				tmp=line.strip()
				#j_tmp=json.loads(json_line)
				if len(tmp)>0:
					m = re.match(self.reg_pattern, line)
					dep.append(m.group(3))

				else:
					#if (len(j_tmp['tokens']) != len(dep)):
					#	print line_num
					#	break
					line_num += 1
					#OUT.write(' '.join(dep)+'\n')
					for out_buf in dep:
						OUT.write(out_buf+'\n')
					dep=[]
	#read dependency struct and dump refined raw corpus
	def read_struct(self):
		refined_input = open(self.arg['output'],'w')
		with open(self.arg['dep_file'], 'r') as IN, open(self.arg['dump_file'], 'w') as OUT:
			dep=[]
			parent=defaultdict(list)
			#paths=[]
			#phrase = []
			#cnt=0
			for line in IN:
				tmp=line.strip()
				#print tmp
				if len(tmp)>0:
					m = re.match(self.reg_pattern, line)
					#if m.group(1)!='punct':
						#print m.group(0)
					dep.append(int(m.group(3)))
					char_buf = m.group(4)
					char_buf = char_buf.replace('-LRB-','(')
					char_buf = char_buf.replace('-RRB-',')')
					refined_input.write(char_buf+' ')
					#phrase.append(m.group(4))

				else:
					refined_input.write('\n')
					cnt=0
					# d start from 1, everything start from 1
					paths = [[] for i in dep]
					#print len(paths)
					for i,d in enumerate(dep):
						parent[d].append(i+1)
					for i in xrange(1, len(dep)+1):
						if i not in parent.keys():
							idx=i
							#cnt = phrase[i-1]
							#paths[idx-1].append(cnt)
							while idx!=0:
								paths[idx-1].append(str(cnt))
								idx=dep[idx-1]
								
							cnt+=1
							#break
					dep=[]
					parent=defaultdict(list)
					out_buf=':'.join(map(lambda x: '_'.join(x), paths))
					OUT.write(out_buf+'\n')
					#break
					#paths=[]
			refined_input.close()

	def analyze(self, json_file):
		cnt=1000
		violat = 0
		all_cnt = 0
		with open(self.arg['dump_file'], 'r') as IN_X, open(json_file,'r') as IN_Y:
			for lineX, lineY in zip(IN_X, IN_Y):
				tmp = json.loads(lineY)
				deps = lineX.strip().split(':')
				if len(tmp['tokens']) != len(deps):
					#print tmp['tokens']
					#print deps
					continue
				for e in tmp['entity_mentions']:
					if e[1] - e[0] > 1 and e[2] == 'RELATION':
						all_cnt += 1
						#print deps[e[0]:e[1]]
						#print tmp['tokens'][e[0]:e[1]]
						dep = sorted(deps[e[0]:e[1]], key = lambda x : x.count('_'))
						if dep[-1].count('_') > 0:
							if dep[-2].count('_') == 0:
								if not min(map(lambda x: x in dep[-1], dep[:-1])):
									violat += 1
									#print deps[e[0]:e[1]]
									#print tmp['tokens'][e[0]:e[1]]
							else:

								for i,d in enumerate(dep[:-1]):
									if d not in dep[i+1]:
										violat += 1
										#print dep[-2].count('_')
										#print max(dep[:-1])
										print deps[e[0]:e[1]]
										print tmp['tokens'][e[0]:e[1]]
										print ' '.join(tmp['tokens'])
										break
				
				if cnt<0:
					break
				cnt-=1
		print str(violat)+'_'+str(all_cnt)

			
		
if __name__ == '__main__':
	tmp=dep_tool({'dep_file': sys.argv[1],'dump_file': sys.argv[2],'output': sys.argv[3]})
	#tmp.read_struct()
	tmp.read_deps()
	#tmp.check_deps()
	#tmp.analyze(sys.argv[4])
