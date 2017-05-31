import json
import sys
import re

class PostProcessor(object):
	"""docstring for PreProcessor"""
	def __init__(self):
		self.prediction=[]
		self.groundtruth=[]
		self.entity_regex=re.compile("<phrase>(.+?)</phrase>")
		self.ner_regex=re.compile("<.+?>(.*?)<\/.+?>")
	
	def loadTest(self,test_file):
		with open(test_file,'r') as IN:
			for line in IN:
				pred=[]
				for item in re.findall(self.entity_regex,line):
					#if ' ' in item:
					#if ' ' in item.strip():
					pred.append(item.strip())
				self.prediction.append(pred)
		#print self.prediction
	def loadNER(self,test_file):
		with open(test_file,'r') as IN:
			for line in IN:
				pred=[]
				for item in re.findall(self.ner_regex,line):
					#if ' ' in item:
					pred.append(item.strip().lower())
				self.prediction.append(pred)
		#print self.prediction

	def loadGroundTruth(self, groundtruth_file):
		with open(groundtruth_file,'r') as IN:
			for line in IN:
				gt=[]
				content=json.loads(line)
				for e in content['entityMentions']:
					#if e[1]-e[0]>0:
					#gt.append(' '.join(content['tokens'][e['end']:e['start']]).strip())
					#e[0]-1 in kbp wenqi's run
					gt.append(' '.join(content['tokens'][e[0]-1:e[1]]).strip())
				self.groundtruth.append(gt)
		#
		#print self.groundtruth

	def getMetrics(self):
		true_pos=0.0
		all_pos=0
		all_pred=0
		veri_pos=0
		veri_pred=0
		for i,j in zip(self.groundtruth,self.prediction):
			all_pos+=len(i)
			all_pred+=len(j)
			#veri_pos+=len(set(i))
			#veri_pred+=len(set(j))
			true_pos+=len(set(i)&set(j))
		#print true_pos,all_pos,all_pred #veri_pred,veri_pos
		print true_pos/all_pos,true_pos/all_pred

		
if __name__ == '__main__':
	tmp=PostProcessor()
	tmp.loadGroundTruth(sys.argv[1])
	tmp.loadTest(sys.argv[2])
	#tmp.loadNER(sys.argv[2])
	tmp.getMetrics()