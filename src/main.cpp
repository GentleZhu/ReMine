#include "utils/parameters.h"
#include "utils/commandline_flags.h"
#include "utils/utils.h"
#include "frequent_pattern_mining/frequent_pattern_mining.h"
#include "data/documents.h"
#include "classification/feature_extraction.h"
#include "classification/label_generation.h"
#include "classification/predict_quality.h"
#include "model_training/segmentation.h"
#include "data/dump.h"
#include "query/parser.h"
#include <fstream>

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;
using FrequentPatternMining::patterns_tag;
using FrequentPatternMining::pattern2id_tag;
using Documents::posid2Tag;

int main(int argc, char* argv[])
{
    parseCommandFlags(argc, argv);

    sscanf(argv[1], "%d", &NTHREADS);
    omp_set_num_threads(NTHREADS);

    // load stopwords, documents, and capital information
    Documents::loadStopwords(STOPWORDS_FILE);
    Documents::loadAllTrainingFiles(TRAIN_FILE, POS_TAGS_FILE, TRAIN_CAPITAL_FILE);
    //cerr<<"here"<<endl;
    Documents::splitIntoSentences();

    FrequentPatternMining::mine(MIN_SUP, MAX_LEN);
    // check the patterns
    if (INTERMEDIATE) {
        vector<pair<int, int>> order;
        for (int i = 0; i < patterns.size(); ++ i) {
            order.push_back(make_pair(patterns[i].currentFreq, i));
        }
        Dump::dumpRankingList("tmp_remine/frequent_patterns.txt", order);
    }

    FrequentPatternMining::mine_pos(MIN_SUP, MAX_LEN);
    //vector<int> selected = Label::generate_samples(QUALITY_FILE_TAG);
    // feature extraction
    vector<string> featureNamesPhrase;
    vector<vector<double>> featuresPhrase = Features::extract(featureNamesPhrase);

    vector<string> featureNamesSemantic;
    vector<vector<double>> featuresSemantic = Features::extract_tag(featureNamesSemantic);

    vector<string> featureNamesUnigram;
    vector<vector<double>> featuresUnigram = Features::extractUnigram(featureNamesUnigram);


    cerr << "feature extraction done!" << endl;
    // return -1;
    vector<Pattern> phrase_truth;
    vector<Pattern> entity_truth;
    vector<Pattern> relation_truth;
    if (LABEL_FILE != "") {
        cerr << "=== Load Existing Labels ===" << endl;
        phrase_truth = Label::loadLabels(LABEL_FILE);
        int recognized = Features::recognize(phrase_truth);
    } else {
        // generate labels
        cerr << "=== Generate Phrase Labels ===" << endl;
        // multi-words
        cerr << QUALITY_FILE <<endl;
        phrase_truth = Label::generate(featuresPhrase, featureNamesPhrase, ALL_FILE, QUALITY_FILE);
        int recognized = Features::recognize(phrase_truth);

        
        // semantic labels come from
        cerr << "=== Generate Semantic Labels ===" << endl;
        entity_truth = Label::generate_tag(featuresSemantic, QUALITY_FILE_ENTITY,true);
        int recognized_entity = Features::recognize_tag(entity_truth);
        //cout<<"*****recognized entity "<<recognized_entity<<endl;
        relation_truth = Label::generate_tag(featuresSemantic, QUALITY_FILE_RELATION,false);
        int recognized_relation = Features::recognize_tag(relation_truth);

        if (POSTAG_SCORE == 1){ 
        featuresSemantic = Features::extract_tag(featureNamesSemantic);
        cerr<<"Hi Bran, look at here"<<endl;
        }

        // unigram
        cerr << "=== Generate Unigram Labels ===" << endl;
        vector<Pattern> truthUnigram = Label::generateUnigram(ALL_FILE, QUALITY_FILE);
        int recognizedUnigram = Features::recognize(truthUnigram);

        if (INTERMEDIATE) {
            Dump::dumpLabels("tmp_remine/generated_label.txt", phrase_truth);
            Dump::dumpLabels("tmp_remine/generated_unigram_label.txt", truthUnigram);

            Dump::dumpFeatures("tmp_remine/features_for_labels.tsv", featuresPhrase, entity_truth);
            Dump::dumpFeatures("tmp_remine/features_for_unigram_labels.tsv", featuresUnigram, truthUnigram);
        }
        //return -1;
    }

    
    //return -1;
    //Parser remine;
    if (ENABLE_POS_TAGGING) {
        Segmentation::initializePosTags(Documents::posTag2id.size());
    }

    // SegPhrase, +, ++, +++, ...
    for (int iteration = 0; iteration < ITERATIONS; ++ iteration) {
        fprintf(stderr, "Feature Matrix = %d X %d\n", featuresPhrase.size(), featuresPhrase.back().size());
        fprintf(stderr, "Tag Feature Matrix = %d X %d\n", featuresSemantic.size(), featuresSemantic.back().size());
        predictQuality(patterns, featuresPhrase, featureNamesPhrase);
        predictQualityUnigram(patterns, featuresUnigram, featureNamesUnigram);
        if (POSTAG_SCORE == 1){
            //cerr<<featureNames_tag.size()<<endl;
            //cerr<<features_tag.back().size()<<endl;
            predictPosTagQuality(patterns_tag, featuresSemantic, featureNamesSemantic, true);
            predictPosTagQuality(patterns_tag, featuresSemantic, featureNamesSemantic, false);
            combineScore(patterns,patterns_tag,pattern2id_tag);
            cerr<<"predict in the loop"<<endl;
        }
        constructTrie(); // update the current frequent enough patterns
        constructTrie_pos(); //
        //break;
        // check the quality
        if (INTERMEDIATE) {
            char filename[256];
            sprintf(filename, "tmp_remine/iter_%d_quality", iteration);
            Dump::dumpResults(filename);
        }

        if (!ENABLE_POS_TAGGING) {
            cerr << "[Length Penalty Mode]" << endl;
            double penalty = EPS;
            if (true) {
                // Binary Search for Length Penalty
                double lower = EPS, upper = 200;
                for (int _ = 0; _ < 10; ++ _) {
                    penalty = (lower + upper) / 2;
                    Segmentation segmentation(penalty);
                    segmentation.rectifyFrequency(Documents::sentences);
                    double wrong = 0, total = 0;
                    # pragma omp parallel for reduction (+:total,wrong)
                    for (int i = 0; i < phrase_truth.size(); ++ i) {
                        if (phrase_truth[i].label == 1) {
                            ++ total;
                            vector<double> f;
                            vector<int> pre;
                            segmentation.viterbi(phrase_truth[i].tokens, f, pre);
                            wrong += pre[phrase_truth[i].tokens.size()] != 0;
                        }
                    }
                    if (wrong / total <= DISCARD) {
                        lower = penalty;
                    } else {
                        upper = penalty;
                    }
                }
                cerr << "Length Penalty = " << penalty << endl;
            }
            // Running Segmentation
            Segmentation segmentation(penalty);
            segmentation.rectifyFrequency(Documents::sentences);
        } else {
            cerr << "[POS Tags Mode]" << endl;
            if (true) {
                Segmentation segmentation(ENABLE_POS_TAGGING);
                double last = 1e100;
                for (int inner = 0; inner < 10; ++ inner) {
                    double energy = segmentation.adjustPOSTagTransition(Documents::sentences, MIN_SUP);
                    if (fabs(energy - last) / fabs(last) < EPS) {
                        break;
                    }
                    last = energy;
                }
            }

            if (INTERMEDIATE) {
                char filename[256];
                sprintf(filename, "tmp/iter_%d_pos_tags.txt", iteration);
                Dump::dumpPOSTransition(filename);
            }

            Segmentation segmentation(ENABLE_POS_TAGGING);
            segmentation.rectifyFrequencyPOS(Documents::sentences, MIN_SUP);

            //if (iteration==ITERATIONS-1){
            //    cerr<<"Initialize the probability!"<<endl;
            //    remine.initialize(segmentation.getProb(),segmentation.getSize());
            //}
        }

        if (iteration + 1 < ITERATIONS) {
            // rectify the features
            cerr << "Rectify Features..." << endl;
            Label::removeWrongLabels();

            /*
            // use number of sentences + rectified frequency to approximate the new idf
            double docs = Documents::sentences.size() + EPS;
            double diff = 0;
            int cnt = 0;
            for (int i = 0; i < patterns.size(); ++ i) {
                if (patterns[i].size() == 1) {
                    const TOKEN_ID_TYPE& token = patterns[i].tokens[0];
                    TOTAL_TOKENS_TYPE freq = patterns[i].currentFreq;
                    double newIdf = log(docs / (freq + EPS) + EPS);
                    diff += abs(newIdf - Documents::idf[token]);
                    ++ cnt;
                    Documents::idf[token] = newIdf;
                }
            }
            */

            featuresPhrase = Features::extract(featureNamesPhrase);
            featuresUnigram = Features::extractUnigram(featureNamesUnigram);
            if (POSTAG_SCORE == 1){
                //cerr<<"here"<<endl;
                featuresSemantic = Features::extract_tag(featureNamesSemantic);
            }
        }

        // check the quality
        if (INTERMEDIATE) {
            char filename[256];
            sprintf(filename, "tmp/iter_%d_frequent_quality", iteration);
            Dump::dumpResults(filename);
        }

    }

    /*
    if (POSTAG_SCORE == 2){
        cerr<<"[POS TAG CLASSIFIER IS ON]"<<endl;
        predictPosTagQuality(patterns_tag, features_tag, featureNames_tag);
        //predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, patterns, pattern2id_tag);
        //predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, selected);
    }

    if (POSTAG_SCORE > 0){
        combineScore(patterns,patterns_tag,pattern2id_tag);
        predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, selected);
    }
    */

    /*
    if (INTERMEDIATE) {
        //FILE* out = tryOpen("tmp/quality_phrases.txt", "w");
        fstream fout;
        fout.open("tmp/distribution.txt", fstream::out);

        for (int i = 0; i < selected.size(); ++ i) {
            int iter = selected[i];
            fout<<patterns_tag[iter].quality<<endl;
        }
        fout.close();
    }
    
    unordered_set<int> dict;
    for (int i = 0; i < patterns.size(); ++ i) {
        //change this line
        if (patterns[i].size() > 1 && patterns[i].currentFreq > 0) {
            //order.push_back(make_pair(patterns[i].quality, i));
            if (patterns[i].quality > 0.1)
                dict.insert(i);
        }
    }
    */

    /*

    vector<vector<pair<string, int>>> parse_result = remine.loadTestingFiles(TEST_FILE, dict);
    //vector<vector<string>> parse_result = Parser::loadTestingFiles(TEST_FILE);
    vector<Pattern> parse_patterns; 

    for (int i = 0; i < parse_result.size(); ++i){
        for (int ii = 0; ii < parse_result[i].size(); ++ii){
            //fout << parse_result[i][ii].first << endl;
            parse_patterns.push_back(patterns[parse_result[i][ii].second]);
        }
    }

    //predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, parse_patterns, pattern2id_tag);
    if (true) {
        fstream fout;
        fout.open("tmp/test_token.txt", fstream::out);
        //FILE* out = tryOpen("tmp/test_token.txt", "w");
        //fstream fout;
        //fout.open("tmp/quality_postags.txt", fstream::out);
        int cnt = 0;
        for (int i = 0; i < parse_result.size(); ++i){
            for (int ii = 0; ii < parse_result[i].size(); ++ii){
                fout << parse_result[i][ii].first << '\n' << parse_patterns[cnt ++].quality << endl;
                //parse_patterns.push_back(patterns[parse_result[i][ii].second]);
            }
                //fprintf(out, "%s\t", parse_result[i][ii]);
            fout << "<<<<<<<<<<<<" << endl;
            //fprintf(out, "\n");
        }
        //fclose(out);
        fout.close();
    }
    */

    Dump::dumpResults("tmp_remine/final_quality");
    Dump::dumpSegmentationModel("results_remine/segmentation.model");

    return 0;
}