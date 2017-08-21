#Parsing

#Refine input

#NER
ROOT=/shared/data/qiz3/ReMine/
RAW_INPUT=$1
NER_OUTPUT=$2
TAG_OUTPUT=$3
touch $ROOT$NER_OUTPUT
cd baselines/stanford-ner-2016-10-31
./ner.sh $ROOT$RAW_INPUT > $ROOT$NER_OUTPUT
cd .. ..

python src_py/extract_entity.py data_remine/nyt_6k_ner.txt data_remine/nyt_6k_ner.dump

cd utils/stanford-postagger-full-2016-10-31
./stanford-postagger.sh models/english-left3words-distsim.tagger $ROOT$RAW_INPUT > $ROOT$TAG_OUTPUT
cd .. ..

python src_py/utils.py data_remine/nyt_6k_tagged.txt data_remine/nyt_6k_new.json
#python src_py/utils.py 

python src_py/entity_linking.py 

python src_py/entity_linking.py data_remine/nyt_6k_new.json data_remine/nyt_6k_ner.dump data_remine/nyt_6k_ner.dump.type data_remine/nyt_6k_linked.json

#read deps and check violations
python src_py/dep_tools.py x data_remine/nyt_6k_rules.txt y data_remine/nyt_6k_linked.json

#########################
#Distant supervision part

#phrase supervision
## multigram entity/unigram entity/multigram relation/unigram relation
## basically 4X multi-gram entity 2X unigram entity 1X relation 1X relation unigram
#pos tag supervision
python src_py/utils relationLinker $JSON_INPUT tmp_remine/pos_relation_token.txt /shared/data/qiz3/ReMine/data_remine/nyt_6k_

shuf -n 4000 x >> quality.phrase

python ../src_py/PreProcessor.py translate ../data_remine/nyt_6k_linked.json tokenized_train.txt ../data_remine/nyt_6k_linked.json tokenized_text_to_seg.txt ../data/EN/stopwords.txt tokenized_stopwords.txt ../data_remine/nyt_6k_quality.txt tokenized_quality.txt

./remine_exp.sh

./remine_seg.sh

#Translate ReMine Result
python src_py/extract_phrases.py 
