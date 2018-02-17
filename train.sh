mkdir -p tmp/
echo ${green}===Entity Linking===${reset}
python3 src_py/distantSupervision.py --op=entityExtractor --in1=data/nyt/train_nyt.json --out=tmp/nyt.entities
python3 src_py/distantSupervision.py --op=relationLinker --in1=data/nyt/train_nyt.json --in2=pickle_ --out=tmp/nyt.relations

mkdir -p tmp_remine/
echo ${green}===Tokenizaztion===${reset}
#python3 src_py/preprocessing.py --op=train --in1=$TWT_DIR/lemmasTotal.txt --in2=$TWT_DIR/posTotal.txt --in3=$TWT_DIR/depTotal.txt
python3 src_py/preprocessing.py --op=train --in1=data/nyt/total.lemmas.txt --in2=data/nyt/total.pos.txt --in3=data/nyt/total.dep.txt
#python3 src_py/preprocessing.py --op=test --in1=$KBP_DIR/test.lemmas.txt --in2=$KBP_DIR/test.pos.txt --in3=$KBP_DIR/test.dep.txt
python3 src_py/preprocessing.py --op=test --in1=data/nyt/test.lemmas.txt --in2=data/nyt/test.pos.txt --in3=data/nyt/test.dep.txt
#python3 src_py/preprocessing.py --op=chunk --in1=$TWT_DIR/lemmasTotal.txt --in2=$TWT_DIR/posTotal.txt
python3 src_py/preprocessing.py --op=chunk --in1=data/nyt/total.lemmas.txt --in2=data/nyt/total.pos.txt
python3 src_py/preprocessing.py --op=translate --in1=data/stopwords.txt --out=tmp_remine/tokenized_stopwords.txt

python3 src_py/preprocessing.py --op=translate --in1=tmp/nyt.entities --out=tmp_remine/tokenized_quality.txt

python3 src_py/preprocessing.py --op=translate --in1=tmp/nyt.relations --out=tmp_remine/tokenized_negatives.txt