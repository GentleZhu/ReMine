# ReMine
Take NYT13_110K dataset as a example:
## Pipeline
1> Input: Pre-defined pos_tag relation patterns, Entity Seeds

2> Use Entity Seeds to generate entity pos tag patterns, pos_tag relation patterns generate relation phrases. Filter out low-frequency entity
postag seqs and relation phrases

3> Generate phrases supervision by combing unigram/multi-gram entity/multi-gram relation. <b>Tune</b>

4> Preprocess Training/Testing Corpus, using python src_py/Preprocessor.py translation mode

5> Run ReMine, given phrases/entity postags/relation postags as supervision. Segmentation lable includes <None>/<ENTITY>/<RELATION>

6> Run ReMine segmentation, postprocess using Preprocessor.py segmentation mode

## Issues
1. Unigram occupies top of the ranklist, <b>Potential Solution</b>:Treat unigram/entity/relation differently

2. Not able to handle unigram relation yet, <b>discussion</b>

3. Take time to check whole pipeline and run case study for entity detection and relation detection. There may exist some bugs in current version

4. Ideal generative Process should be able to denoise/reduce search space in order to compete with OpenIE system: Ollie/Stanford IE/ReVerb
