/*
Use dumped patterns as initialization
Tune the segmentation model under RM mode
*/

#include "utils/commandline_flags.h"
#include "utils/utils.h"
#include "frequent_pattern_mining/frequent_pattern_mining.h"
#include "data/documents.h"
#include "classification/feature_extraction.h"
#include "classification/label_generation.h"
#include "classification/predict_quality.h"
#include "model_training/segmentation.h"
#include "data/dump.h"

int main(int argc, char* argv[])
{
	parseCommandFlags(argc, argv);

    sscanf(argv[1], "%d", &NTHREADS);
    omp_set_num_threads(NTHREADS);

    // load stopwords, documents, and capital information
    Documents::loadStopwords(STOPWORDS_FILE);
    if (ENABLE_POS_TAGGING) {
        Documents::loadPuncwords(PUNC_FILE);
    }
    Documents::loadAllTrainingFiles(TRAIN_FILE, POS_TAGS_FILE, TRAIN_CAPITAL_FILE, TRAIN_DEPS_FILE);
    
    Documents::splitIntoSentences(ORIGINAL_PUNC, TRAIN_FILE);

 	Dump::loadSegmentationModel(SEGMENTATION_MODEL);

 	constructTrie();

 	Segmentation* segmenter;
 	if (ENABLE_POS_TAGGING) {
 		segmenter = new Segmentation(ENABLE_POS_TAGGING);
 	} else {
 		segmenter = new Segmentation(Segmentation::penalty);
 	}

 	cerr << "[Constraints Model]" << endl;
	for (int inner = 0; inner < 10; ++ inner) {
	        double energy = segmentation.adjustConstraints(Documents::sentences, MIN_SUP);
	        if (fabs(energy - last) / fabs(last) < EPS) {
	            break;
	        }
	        last = energy;
	}

	Segmentation segmentation(ENABLE_POS_TAGGING);
    segmentation.rectifyFrequencyDeps(Documents::sentences);




}