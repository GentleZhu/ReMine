# -*- coding: utf-8 -*-
"""
Created on Sat Apr 21 22:59:41 2018

@author: Xiaolu
"""

import json
pos = "tmp/Sample_wiki.pos.txt"
tokens = "tmp/Sample_wiki_index.txt"
pos_lines = open(pos,'r',encoding = 'UTF-8').readlines()
pos_list = [line.split() for line in pos_lines]
tokens_lines = open(tokens,'r',encoding ='UTF-8').readlines()
tokens_list = [line.strip().split() for line in tokens_lines]

result=[]


for index in range(len(pos_list)):
    result.append({'pos':pos_list[index],'tokens':tokens_list[index]})
    
file =  open("wiki.json","w",encoding ='UTF-8')
for index in result:
    file.write(json.dumps(index)+'\n')
file.close()   

