import sys
import argparse
import utils
#import EntityLinker_freebase

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description="Run node2vec.")
	parser.add_argument('--in1', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')
	parser.add_argument('--in2', nargs='?', default='graph/karate.edgelist',
	                    help='Input graph path')

	parser.add_argument('--out', nargs='?', default='emb/karate.emb',
	                    help='Embeddings path')
	parser.add_argument('--opt', type=bool, default=False,
	                    help='Embeddings path')

	parser.add_argument('--op', help='Type of supervision')


	args = parser.parse_args()

	if args.op == 'relationLinker':
		utils.relationLinker(args.in1, args.in2)
		utils.dumpRelations(args.in2, args.out, False)

	elif args.op == 'entityExtractor':
		utils.getEntity(args.in1, args.out, args.opt)

	elif args.op == 'entityLinker':
		utils.entityLinker(args.in1, args.in2, args.out)

	elif args.op == 'exe':
		tmp_out = 'tmp/train_annotated.json'
		tmp_ent = 'tmp/nyt.entities'
		tmp_rm = 'tmp/nyt.relations'
		utils.entityLinker(args.in1, args.in2, tmp_out)
		utils.getEntity(tmp_out, tmp_ent, args.opt)
		utils.relationLinker(tmp_out, 'pickle')
		utils.dumpRelations('pickle', tmp_rm, False)