green=`tput setaf 2`
reset=`tput sgr0`

mkdir -p tmp/
echo ${green}===Entity Linking===${reset}
python3 src_py/distantSupervision.py --op=entityExtractor --in1=data/nyt/train_nyt.json --out=tmp/nyt.entities
python3 src_py/distantSupervision.py --op=relationLinker --in1=data/nyt/train_nyt.json --in2=pickle_ --out=tmp/nyt.relations

mkdir -p tmp_remine/
echo ${green}===Tokenizaztion===${reset}
python3 src_py/preprocessing.py --op=train --in1=data/nyt/total.lemmas.txt --in2=data/nyt/total.pos.txt --in3=data/nyt/total.dep.txt
python3 src_py/preprocessing.py --op=test --in1=data/nyt/test.lemmas.txt --in2=data/nyt/test.pos.txt --in3=data/nyt/test.dep.txt
python3 src_py/preprocessing.py --op=chunk --in1=data/nyt/total.lemmas.txt --in2=data/nyt/total.pos.txt
python3 src_py/preprocessing.py --op=translate --in1=data/stopwords.txt --out=tmp_remine/tokenized_stopwords.txt

python3 src_py/preprocessing.py --op=translate --in1=tmp/nyt.entities --out=tmp_remine/tokenized_quality.txt

python3 src_py/preprocessing.py --op=translate --in1=tmp/nyt.relations --out=tmp_remine/tokenized_negatives.txt

echo ${green}===Phrase Extraction===${reset}
bash phrase_extraction.sh
python3 src_py/preprocessing.py --op=segment --in1=tmp_remine/tokenized_segmented_sentences.txt --out=results_remine/segmentation.txt

python3 src_py/postprocessing.py  --op=extract --in1=results_remine/segmentation.txt --in2=data/nyt/total.lemmas.txt --in3=data/nyt/total.pos.txt --out1=tmp/total.json
python3 src_py/postprocessing.py  --op=transformat --in1=tmp/total.json --out1=tmp_remine/entity_position.txt

./bin/tuple_generation_train --verbose --thread 10 --max_positives -1 --model results_remine/segmentation.model --pos_tag
TOKEN_MAPPING=tmp_remine/token_mapping.txt
TOKENIZER="-cp .:tools/tokenizer/lib/*:tools/tokenizer/resources/:tools/tokenizer/build/ Tokenizer"
java $TOKENIZER -m translate -l EN -c N -i tmp_remine/rm_tokenized_train.txt -o tmp_remine/rm_segmentation.txt -t $TOKEN_MAPPING
#./bin/genSepath data/nyt/total.dep_2.txt tmp_remine/entity_position.txt data/nyt/total.pos.txt tmp_remine/shortest_paths.txt
#python3 src_py/postprocessing.py --op=generatepath --in1=tmp/total.json --in2=tmp_remine/shortest_paths.txt --in3=data/nyt/total.dep_2.txt --out1=tmp/total_rm.json --out2=tmp_remine/rm_deps_train.txt > tmp_remine/remine_test_part_b.txt
