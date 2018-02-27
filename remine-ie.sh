green=`tput setaf 2`
red=`tput setaf 1`
blue=`tput setaf 3`
reset=`tput sgr0`

mkdir -p tmp_remine/
mkdir -p tmp/
mkdir -p results_remine/

echo ${green}===Tokenizaztion===${reset}
python3 src_py/preprocessing.py --op=test --in1=data/nyt/test.lemmas.txt --in2=data/nyt/test.pos.txt --in3=data/nyt/test.dep.txt
echo ${blue}===Extracting Phrases===${reset}
./bin/remine --input_file tmp_remine/tokenized_test.txt --pos_file tmp_remine/pos_tags_test.txt --deps_file tmp_remine/deps_test.txt --model pre_train/segmentation.model --mode 0
python3 src_py/preprocessing.py --op=segment_test --in1=tmp_remine/remine_tokenized_segmented_sentences.txt --out=results_remine/remine_segmentation.txt
python3 src_py/postprocessing.py  --op=extract --in1=results_remine/remine_segmentation.txt --in2=data/nyt/test.lemmas.txt --in3=data/nyt/test.pos.txt --out1=tmp/test.json
python3 src_py/postprocessing.py  --op=transformat --in1=tmp/test.json --out1=tmp_remine/remine_entity_position.txt
echo ${red}===Extracting Tuples===${reset}
./bin/remine --input_file tmp_remine/tokenized_test.txt --pos_file tmp_remine/pos_tags_test.txt --deps_file tmp_remine/deps_test.txt --ems_file tmp_remine/remine_entity_position.txt --model pre_train/segmentation.model --mode 1
TOKEN_MAPPING=tmp_remine/token_mapping.txt
TOKENIZER="-cp .:tools/tokenizer/lib/*:tools/tokenizer/resources/:tools/tokenizer/build/ Tokenizer"
echo ${green}===Output===${reset}
java $TOKENIZER -m translate -l EN -c N -i tmp_remine/remine_tokenized_segmented_sentences.txt -o results_remine/remine_result.txt -t $TOKEN_MAPPING
