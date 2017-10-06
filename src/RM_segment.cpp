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

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;

int main(int argc, char* argv[])
{
	parseCommandFlags(argc, argv);

    sscanf(argv[1], "%d", &NTHREADS);
    omp_set_num_threads(NTHREADS);

    // load stopwords, documents, and capital information
    Documents::loadStopwords(RM_STOPWORDS_FILE);
    if (ENABLE_POS_TAGGING) {
        Documents::loadPuncwords(RM_PUNC_FILE);
    }
    Documents::loadAllTrainingFiles(RM_TRAIN_FILE, RM_POS_TAGS_FILE, RM_TRAIN_CAPITAL_FILE, RM_TRAIN_DEPS_FILE);
    
    Documents::splitIntoSentences(ORIGINAL_PUNC, RM_TRAIN_FILE);

 	Dump::loadSegmentationModel(SEGMENTATION_MODEL);

 	cerr << "Extracting features..." << endl;
    Features::loadPosgroup("tmp_remine/pos_tag.map");


    vector<string> featureNamesPhrase = {"stat_f1", "stat_f2", "stat_f4", "stat_outside",
                        "punc_quote", "punc_dash", "punc_parenthesis", "first_capitalized",
                        // "all_capitalized",
                        "stopwords_1st", "stopwords_last", "stopwords_ratio", "avg_idf",
                        "complete_sub", "complete_super",
                        "CC", "CD", "DT", "IN", "ADJ", "NP",
                        "PP", "ADV", "VB", "WH", "NA",
                        };;
    vector<vector<double>> featuresPhrase = Dump::loadFeatures("tmp_remine/phrase.feat", featureNamesPhrase);
    // vector<vector<double>> featuresPhrase = Features::extract(featureNamesPhrase);

    // vector<string> featureNamesSemantic;
    // vector<vector<double>> featuresSemantic = Features::extract_tag(featureNamesSemantic);
    vector<string> featureNamesUnigram = {"log_frequency", "independent_ratio",
                        "stopwords", "idf",
                        "punc_quote", "punc_parenthesis", "first_capitalized", "all_capitalized",
                        "complete_super", "extrabit_noun", "extrabit_verb"
                        };
    vector<vector<double>> featuresUnigram = Dump::loadFeaturesUnigram("tmp_remine/unigram.feat", featureNamesUnigram);
    // vector<vector<double>> featuresUnigram = Features::extractUnigram(featureNamesUnigram);
 	
 	cerr << "=== Generate Phrase Labels ===" << endl;
 	vector<Pattern> phrase_truth = Label::generateAll(LABEL_METHOD, LABEL_FILE, RM_QUALITY_FILE, RM_NEGATIVES_FILE);

 	// This one is required!
 	// Use the same tokenization mapping.

 	
 	
 	if (ENABLE_POS_TAGGING) {
        Segmentation::initializePosTags(Documents::posTag2id.size());
        Segmentation::initializeDeps(Documents::sentences, MAX_LEN);        
    }
    

    predictQuality(patterns, featuresPhrase, featureNamesPhrase);
    // predictQualityUnigram(patterns, featuresUnigram, featureNamesUnigram);

 	constructTrie();

	for (int iteration = 0; iteration < ITERATIONS; ++ iteration) {
	 	if (true) {
		 	Segmentation segmentation(ENABLE_POS_TAGGING);

		 	cerr << "[Constraints Model]" << endl;
		 	double last = 1e100;
			for (int inner = 0; inner < 10; ++ inner) {
			        double energy = segmentation.adjustConstraints(Documents::sentences, MIN_SUP);
			        if (fabs(energy - last) / fabs(last) < EPS) {
			            break;
			        }
			        last = energy;
			}
		}

		Segmentation segmentation(ENABLE_POS_TAGGING);
	    segmentation.rectifyFrequencyDeps(Documents::sentences);
	}

    // featuresPhrase = Features::extract(featureNamesPhrase);
    // featuresUnigram = Features::extractUnigram(featureNamesUnigram);

    Dump::dumpResults("tmp_remine/final_quality");
    Dump::dumpSegmentationModel("results_remine/rm_segmentation.model");

    return 0;
}