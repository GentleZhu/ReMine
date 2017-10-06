#java -cp corpus-processor.jar nlptools.SentenceAnnotator /shared/data/qiz3/data/nyt/train.txt /shared/data/qiz3/data/nyt/train

echo ${green}===Tokenizaztion===${reset}
python3 src_py/preprocessing.py --op=train --in1=/shared/data/qiz3/data/nyt/train.lemmas.txt --in2=/shared/data/qiz3/data/nyt/train.pos.txt --in3=/shared/data/qiz3/data/nyt/train.dep.txt
python3 src_py/preprocessing.py --op=chunk --in1=/shared/data/qiz3/data/nyt/train.lemmas.txt --in2=/shared/data/qiz3/data/nyt/train.pos.txt
python3 src_py/preprocessing.py --op=translate --in1=data/EN/stopwords.txt --out=tmp_remine/tokenized_stopwords.txt
python3 src_py/preprocessing.py --op=translate --in1=data_remine/nyt_6k_quality.txt --out=tmp_remine/tokenized_quality.txt
python3 src_py/preprocessing.py --op=translate --in1=data_remine/nyt_6k_negatives.txt --out=tmp_remine/tokenized_negatives.txt

bash remine_exp.sh
bash remine_seg.sh

python3 src_py/preprocessing.py --op=segment --in1=tmp_remine/tokenized_segmented_sentences.txt --out=results_remine/segmentation.txt

echo ${green}===Entity Mining===${reset}
python3 src_py/postprocessing.py  --op=extract --in1=results_remine/segmentation.txt --in2=/shared/data/qiz3/data/nyt/train.lemmas.txt --out=remine_extraction/ver2/train.json
python3 src_py/postprocessing.py  --op=transformat --in1=remine_extraction/ver2/train.json --out=remine_extraction/ver2/entity_position.txt


echo ${green}===Relation Mining[Local Optimization]===${reset}


echo ${green}===Knowledge Base Construction[Global Optimization]===${reset}