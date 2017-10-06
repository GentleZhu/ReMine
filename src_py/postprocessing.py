import sys
import argparse,json

class PostProcessor(object):
	def extract(self,test_file,json_file,output):
		with open(test_file,'r') as IN, open(json_file, 'r') as IN_JSON, open(output,'w') as OUT:
			e_not_found = 0
			r_not_found = 0
			for line, json_line in zip(IN, IN_JSON):
				pred=[]
				pred_rm = []
				for item in line.split(']_['):
					if ':EP' in item:
						pred.append(item.rstrip(' :EP').strip().replace('(','-LRB-').replace(')','-RRB-'))
				tmp = {}
				tmp['tokens'] = json_line.strip().split(' ')
				#exists = set()
				cur_max = dict()
				tmp['entityMentions'] = []
				ptr = 0
				for e in pred:
					window_size=e.count(' ') + 1
				
					found=False
					#ptr = 0
					while ptr+window_size <= len(tmp['tokens']):
						if ' '.join(tmp['tokens'][ptr:ptr+window_size]) == e:
							found=True
							break
						ptr+=1
					#if found and (ptr, ptr+window_size) not in exists:
					if not found:
						ptr = 0 
						e_not_found += 1
					if found:
						if ptr+window_size not in cur_max:
							cur_max[ptr+window_size] = ptr
							#tmp['entityMentions'].append([ptr, ptr+window_size, e])
						ptr+=window_size
				#keys = cur_max.keys().sort(reverse=True)
				ptr = len(tmp['tokens'])
				while ptr > 0:
					if ptr in cur_max:
						tmp['entityMentions'].append([cur_max[ptr], ptr, ' '.join(tmp['tokens'][cur_max[ptr]:ptr])])
						ptr = cur_max[ptr]
					else:
						ptr -= 1
					
				#tmp['entityMentions'] = list(exists)
				tmp['entityMentions'].sort(key=operator.itemgetter(1))
				OUT.write(json.dumps(tmp) + '\n')
			print("#entity not found:",e_not_found)

	def transformat(self, file_path, output):
		with open(file_path) as IN, open(output, 'w') as OUT:
			for line in IN:
				tmp = json.loads(line)
				ems = ''
				for em in tmp['entityMentions']:
					if len(em[2]) > 0:
						ems += str(em[0]) + '_'  + str(em[1]) + ' '
				OUT.write(ems.strip()+'\n')


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
	tmp = PostProcessor()
	if args.op == 'extract':
		tmp.transformat(args.in1, args.in2, args.out)
	elif args.op == 'transformat':
		tmp.transformat(args.in1, args.out)