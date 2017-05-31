SEGMENTATION_MODEL=results_remine/segmentation.model
TEXT_TO_SEG=tmp_remine/raw_text_to_seg.txt
#TEXT_TO_SEG=data/remine/nyt_test.txt
#INTERMEDIATE=$(($1 + 0))
HIGHLIGHT_TOP_K=100000
#HIGHLIGHT_TOP_K=$(($INTERMEDIATE * 3000))
ENABLE_POS_TAGGING=0
THREAD=10

green=`tput setaf 2`
reset=`tput sgr0`

echo ${green}===Compilation===${reset}
make all CXX=g++ | grep -v "Nothing to be done for"

mkdir -p tmp_remine
mkdir -p results_remine
### END Compilation###

echo ${green}===Tokenization===${reset}

TOKENIZER="-cp .:tools/tokenizer/lib/*:tools/tokenizer/resources/:tools/tokenizer/build/ Tokenizer"
TOKENIZED_TEXT_TO_SEG=tmp_remine/tokenized_text_to_seg.txt
CASE=tmp_remine/case_tokenized_text_to_seg.txt
TOKEN_MAPPING=tmp_remine/token_mapping.txt
POS_TAGS=tmp_remine/pos_tags_tokenized_text_to_seg.txt

### END Part-Of-Speech Tagging ###

echo ${green}===Segphrasing===${reset}

if [ $ENABLE_POS_TAGGING -eq 1 ]; then
	time ./bin/segphrase_segment \
        --verbose \
        --pos_tag \
        --thread $THREAD \
        --model $SEGMENTATION_MODEL \
		--highlight $HIGHLIGHT_TOP_K
else
	time ./bin/segphrase_segment \
        --verbose \
        --thread $THREAD \
        --model $SEGMENTATION_MODEL \
		--highlight $HIGHLIGHT_TOP_K
fi

### END Segphrasing ###

echo ${green}===Generating Output===${reset}
python src_py/PreProcessor.py segmentation tmp_remine/tokenized_segmented_sentences.txt results_remine/segmentation.txt

#python src_py/PostProcessor.py ../data/nyt/test_new.json results_remine/segmentation.txt #>> $2
#python src_py/PostProcessor.py src_py/kbp_test.json results_remine/segmentation.txt >> $2
#python src_py/PostProcessor.py ../shared_data/data/intermediate/KBP/ntest.json results_remine/segmentation.txt >> $2
#java $TOKENIZER -m translate -i tmp_remine/tokenized_segmented_sentences.txt -o results_remine/segmentation.txt -t $TOKEN_MAPPING -c N -thread $THREAD
#java $TOKENIZER -m segmentation -i $TEXT_TO_SEG -segmented tmp_remine/tokenized_segmented_sentences.txt -o results_remine/segmentation.txt -tokenized tmp_remine/tokenized_text_to_seg.txt

### END Generating Output for Checking Quality ###
