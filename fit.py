
# coding: utf-8

# In[1]:


import sys


main = []
lines = open(sys.argv[0],'r',encoding = 'UTF-8').readlines()
main = [line.strip().split('\t') for line in lines]


# In[2]:



pro=[]
newline = open(sys.argv[1],'r',encoding = 'UTF-8').readlines()
pro=[line.strip().split('\t') for line in newline]


# In[ ]:


res=[]
for i in range(len(main)):
    for j in range(i, len(pro)):
        if (len(main[i]) == len(pro[j])):
            res.push_back(pro[j])
            break
file = open("new_ab.lemmas.txt","w",encoding = 'UTF-8')
for index in range(len(res)):
    file.write(res[index])
file.close()

