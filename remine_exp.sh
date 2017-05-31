FIRST_RUN=1
ENABLE_POS_TAGGING=0
MIN_SUP=20
#echo $MIN_SUP
#0 original SegPhrase, 1 in the loop, 2 at the top
POSTAG_SCORE=1
THREAD=12
#THREAD=1
LABEL_METHOD=ByRandom
MAX_POSITIVES=100
NEGATIVE_RATIO=5
green=`tput setaf 2`
reset=`tput sgr0`


mkdir -p tmp_remine
mkdir -p results_remine

#KBP path
#/shared/data/qiz3/shared_data/data/source/KBP/raw_train.json
#python ../src_py/PreProcessor.py translate ../../data/nyt/train_new.json tokenized_train.txt ../../data/nyt/test_new.json tokenized_text_to_seg.txt ../data/EN/stopwords.txt tokenized_stopwords.txt tokenized_segmented_sentences.txt
#python ../src_py/PreProcessor.py translate /shared/data/qiz3/shared_data/data/source/KBP/raw_train.json tokenized_train.txt /shared/data/qiz3/shared_data/data/intermediate/KBP/ntest.json tokenized_text_to_seg.txt ../data/EN/stopwords.txt tokenized_stopwords.txt
#python src_py/relation_linker.py xxx /shared/data/qiz3/shared_data/data/source/KBP/raw_train.json /shared/data/qiz3/shared_data/data/source/KBP/lf.json
#python ../src_py/PreProcessor.py ../../data/nyt/train_new.json tokenized_train.txt ../../data/nyt/test_new.json tokenized_text_to_seg ../data/EN/stopwords.txt tokenized_stopwords.txt

TOKENIZED_TRAIN=tmp_remine/tokenized_train.txt
TOKENIZED_TEST=tmp_remine/tokenized_test.txt
CASE=tmp_remine/case_tokenized_train.txt
TOKEN_MAPPING=tmp_remine/token_mapping.txt
TOKENIZED_STOPWORDS=tmp_remine/tokenized_stopwords.txt
TOKENIZED_ALL=tmp_remine/refine_postags_quality.txt
#TOKENIZED_ALL=tmp_remine/relation_token.txt
#TOKENIZED_QUALITY=tmp_remine/relation_token.txt
TOKENIZED_QUALITY=tmp_remine/tokenized_quality.txt
POS_TAGS=tmp_remine/pos_tags.txt


echo ${green}===Compilation===${reset}

make all CXX=g++ | grep -v "Nothing to be done for"

echo ${green}===Segphrasing===${reset}

if [ $ENABLE_POS_TAGGING -eq 1 ]; then
	time ./bin/segphrase_train \
        --verbose \
        --pos_tag \
        --thread $THREAD \
        --pos_prune data/BAD_POS_TAGS_PENN.txt \
        --label_method $LABEL_METHOD \
        --max_positives $MAX_POSITIVES \
        --negative_ratio $NEGATIVE_RATIO \
        --postag_score $POSTAG_SCORE \
        --min_sup $MIN_SUP
else
	time ./bin/segphrase_train --verbose --thread $THREAD --label_method $LABEL_METHOD --max_positives $MAX_POSITIVES --negative_ratio $NEGATIVE_RATIO --postag_score $POSTAG_SCORE --min_sup $MIN_SUP
fi

### END Segphrasing ###
echo ${green}===Tokenization===${reset}

TOKENIZER="-cp .:tools/tokenizer/lib/*:tools/tokenizer/resources/:tools/tokenizer/build/ Tokenizer"

java $TOKENIZER -m translate -i tmp_remine/frequent_patterns.txt -o results_remine/frequent_patterns.txt -t $TOKEN_MAPPING -c N -thread $THREAD

echo ${green}===Generating Output===${reset}
java $TOKENIZER -m translate -i tmp_remine/final_quality_multi-words.txt -o results_remine/multi-words_SegPhrase+.txt -t $TOKEN_MAPPING -c N -thread $THREAD
java $TOKENIZER -m translate -i tmp_remine/final_quality_unigrams.txt -o results_remine/unigrams_SegPhrase+.txt -t $TOKEN_MAPPING -c N -thread $THREAD
java $TOKENIZER -m translate -i tmp_remine/final_quality_salient.txt -o results_remine/salient_SegPhrase+.txt -t $TOKEN_MAPPING -c N -thread $THREAD


#java -jar $TOKENIZER -m translate -i tmp/frequent_patterns.txt -o tmp/human_frequent_patterns.txt -t $TOKEN_MAPPING -c N

java $TOKENIZER -m translate -i tmp_remine/generated_label.txt -o results_remine/generated_label.txt -t $TOKEN_MAPPING -c N -thread $THREAD

#for (( i=0; i<2; i++ ))
#do
#   java $TOKENIZER -m translate -i tmp_remine/quality_patterns_iter_$i.txt -o tmp/human_quality_patterns_iter_$i.txt -t $TOKEN_MAPPING -c N -thread $THREAD
#done

### END Generating Output for Checking Quality ###
