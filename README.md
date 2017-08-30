# ReMine
Take NYT13_110K dataset as a example:
## Pipeline
Preprocessing:
1. Link entity seeds 
2. Postagging
3. Relation Extractor(pattern-based)

Multi-Extraction:

1> Generate Pos Tag supervision

2> Generate phrases supervision by combing unigram/multi-gram entity/multi-gram relation. <b>Tune</b>

3> Preprocess Training/Testing Corpus, using python src_py/Preprocessor.py translation mode

4> Run ReMine, given phrases/entity postags/relation postags as supervision. Segmentation lable includes <None>/<ENTITY>/<RELATION>

5> Run ReMine segmentation, postprocess using Preprocessor.py segmentation mode

## Todos
1. Ideal generative Process should be able to denoise/reduce search space in order to compete with OpenIE system: Ollie/Stanford IE/ReVerb

## Bugs
1. Check punctuation boundary case
