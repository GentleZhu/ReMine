FIRST_RUN=0
ENABLE_POS_TAGGING=1
MIN_SUP=10
#echo $MIN_SUP
#0 original SegPhrase, 1 in the loop, 2 at the top
POSTAG_SCORE=1
THREAD=10
#THREAD=1
LABEL_METHOD=DPDN
MAX_POSITIVES=-1
NEGATIVE_RATIO=2
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
#python ../src_py/PreProcessor.py translate ../data_remine/nyt13_entities.json tokenized_train.txt ../data_remine/nyt13_entities.json tokenized_text_to_seg.txt ../data/EN/stopwords.txt tokenized_stopwords.tx

TOKEN_MAPPING=tmp_remine/token_mapping.txt



echo ${green}===Compilation===${reset}

make all CXX=g++ | grep -v "Nothing to be done for"


if [ $FIRST_RUN -eq 1 ]; then
        echo ${green}===Segphrasing===${reset}

        if [ $ENABLE_POS_TAGGING -eq 1 ]; then
        	time ./bin/remine_train \
                --verbose \
                --pos_tag \
                --thread $THREAD \
                --label_method $LABEL_METHOD \
                --max_positives $MAX_POSITIVES \
                --negative_ratio $NEGATIVE_RATIO \
                --postag_score $POSTAG_SCORE \
                --min_sup $MIN_SUP
        else
        	time ./bin/remine_train --verbose --thread $THREAD --label_method $LABEL_METHOD --max_positives $MAX_POSITIVES --negative_ratio $NEGATIVE_RATIO --postag_score $POSTAG_SCORE --min_sup $MIN_SUP
        fi

        ### END Segphrasing ###
        echo ${green}===Tokenization===${reset}

        TOKENIZER="-cp .:tools/tokenizer/lib/*:tools/tokenizer/resources/:tools/tokenizer/build/ Tokenizer"

        java $TOKENIZER -m translate -i tmp_remine/frequent_patterns.txt -o results_remine/frequent_patterns.txt -t $TOKEN_MAPPING -c N -thread $THREAD

        echo ${green}===Generating Output===${reset}
        java $TOKENIZER -m translate -i tmp_remine/final_quality_multi-entities.txt -o results_remine/multigram_entities.txt -t $TOKEN_MAPPING -c N -thread $THREAD
        java $TOKENIZER -m translate -i tmp_remine/final_quality_multi-unigrams.txt -o results_remine/unigrams.txt -t $TOKEN_MAPPING -c N -thread $THREAD
        java $TOKENIZER -m translate -i tmp_remine/final_quality_multi-backgrounds.txt -o results_remine/multigram_backgrounds.txt -t $TOKEN_MAPPING -c N -thread $THREAD
        java $TOKENIZER -m translate -i tmp_remine/final_quality_multi-relations.txt -o results_remine/multigram_relations.txt -t $TOKEN_MAPPING -c N -thread $THREAD
fi

SEGMENTATION_MODEL=results_remine/segmentation.model

if [ $ENABLE_POS_TAGGING -eq 1 ]; then
        time ./bin/remine_segment \
        --verbose \
        --pos_tag \
        --thread $THREAD \
        --model $SEGMENTATION_MODEL 
else
        time ./bin/remine_segment \
        --verbose \
        --thread $THREAD \
        --model $SEGMENTATION_MODEL 
fi

#java -jar $TOKENIZER -m translate -i tmp/frequent_patterns.txt -o tmp/human_frequent_patterns.txt -t $TOKEN_MAPPING -c N

#java $TOKENIZER -m translate -i tmp_remine/generated_label.txt -o results_remine/generated_label.txt -t $TOKEN_MAPPING -c N -thread $THREAD

#for (( i=0; i<2; i++ ))
#do
#   java $TOKENIZER -m translate -i tmp_remine/quality_patterns_iter_$i.txt -o tmp/human_quality_patterns_iter_$i.txt -t $TOKEN_MAPPING -c N -thread $THREAD
#done

### END Generating Output for Checking Quality ###
