import random
#pick = random.sample(range(0, 3000000), 100000)
#text_file = open("tmp/Sample_bio_pubtator.index.txt", "w")
#text_file.write("\n".join([str(x) for x in pick]))
#text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/bio_pubtator/bio_pubtator.tokens.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(5200000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/5.5M_bio_pubtator.token.txt", "w")
text_file.write(output)
text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/bio_pubtator/bio_pubtator.pos.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(5200000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/5.5M_bio_pubtator.pos.txt", "w")
text_file.write(output)
text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/bio_pubtator/bio_pubtator.dep.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(5200000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/5.5M_bio_pubtator.dep.txt", "w")
text_file.write(output)
text_file.close()

#import random
#pick = random.sample(range(0, 300000), 100000)
#text_file = open("tmp/Sample_wiki.index.txt", "w")
#text_file.write("\n".join([str(x) for x in pick]))
#text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/wiki/sampled_wiki.tokens.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(400000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/400K_wiki.token.txt", "w")
text_file.write(output)
text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/wiki/sampled_wiki.pos.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(400000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/400K_wiki.pos.txt", "w")
text_file.write(output)
text_file.close()

output = []
out_str = []
with open('../../../qiz3/data/wiki/sampled_wiki.dep.txt') as fp:
    out_str = fp.read().split('\n')
for i in range(400000):
    output.append(out_str[i])
output = '\n'.join(output)
text_file = open("tmp/400K_wiki.dep.txt", "w")
text_file.write(output)
text_file.close()




