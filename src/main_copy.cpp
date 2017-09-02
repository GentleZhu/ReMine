#include "utils/parameters.h"
#include "utils/commandline_flags.h"
#include "utils/utils.h"
#include "frequent_pattern_mining/frequent_pattern_mining.h"
#include "data/documents.h"
#include "classification/feature_extraction.h"
#include "classification/label_generation.h"
#include "classification/predict_quality.h"
#include "model_training/segmentation.h"
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
    Documents::splitIntoSentences();

    //return -1;
    FrequentPatternMining::mine(MIN_SUP, MAX_LEN);
    // check the patterns
    if (INTERMEDIATE) {
        FILE* out = tryOpen("tmp/frequent_patterns.txt", "w");
        vector<pair<int, int>> order;
        for (int i = 0; i < patterns.size(); ++ i) {
            order.push_back(make_pair(patterns[i].currentFreq, i));
        }
        //sort(order.rbegin(), order.rend());
        for (int iter = 0; iter < order.size(); ++ iter) {
            int i = order[iter].second;
            for (int j = 0; j < patterns[i].tokens.size(); ++ j) {
                fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
            }
        }
        fclose(out);
    }


    vector<string> featureNames;
    vector<vector<double>> features = Features::extract(featureNames);
    fprintf(stderr, "feature extraction done!\n");

    if (INTERMEDIATE) {
        FILE* out = tryOpen("tmp/features.tsv", "w");
        for (int i = 0; i < features.size(); ++ i) {
            if (features[i].size() > 0) {
                for (int j = 0; j < features[i].size(); ++ j) {
                    fprintf(out, "%.10f%c", features[i][j], j + 1 == features[i].size() ? '\n' : '\t');
                }
            }
        }
        fclose(out);
    }


    //cout<<"HereA"<<endl;
    
    
    FrequentPatternMining::mine_pos(30, 8);
    //cout<<"HereB"<<endl;
    // check the patterns

    if (INTERMEDIATE) {
        //FILE* out = tryOpen("tmp/frequent_patterns_tag.txt", "w");
        fstream fout;
        fout.open("tmp/frequent_patterns_tag.txt", fstream::out);
        vector<pair<int, int>> order;
        for (int i = 0; i < patterns_tag.size(); ++ i) {
            order.push_back(make_pair(patterns_tag[i].currentFreq, i));
        }
        //sort(order.rbegin(), order.rend());
        for (int iter = 0; iter < order.size(); ++ iter) {
            int i = order[iter].second;
            for (int j = 0; j < patterns_tag[i].tokens.size(); ++ j) {
                //cout<<posid2Tag.find(patterns_tag[i].tokens[j])->second<<endl;
                fout << posid2Tag.find(patterns_tag[i].tokens[j])->second;
                if (j + 1 == patterns_tag[i].tokens.size())
                    fout<<endl;
                else
                    fout<<" ";
                //fprintf(out, "%d%c", patterns_tag[i].tokens[j], j + 1 == patterns_tag[i].tokens.size() ? '\n' : ' ');
            }
        }
        fout.close();
        //fclose(out);
    }

    //return -1;

    vector<string> featureNames_tag;
    vector<vector<double>> features_tag = Features::extract_tag(featureNames_tag);
    fprintf(stderr, "postag feature extraction done!\n");

    if (INTERMEDIATE) {
        FILE* out = tryOpen("tmp/features_tag.tsv", "w");
        for (int i = 0; i < features_tag.size(); ++ i) {
            if (features_tag[i].size() > 0) {
                for (int j = 0; j < features_tag[i].size(); ++ j) {
                    fprintf(out, "%.10f%c", features_tag[i][j], j + 1 == features_tag[i].size() ? '\n' : '\t');
                }
            }
        }
        fclose(out);
    }

    //return -1;

    vector<Pattern> truth;
    if (LABEL_FILE != "") {
        cerr << "=== Load Existing Labels ===" << endl;
        truth = Label::loadLabels(LABEL_FILE);
    } else {
        // generate labels
        cerr << "=== Generate Labels ===" << endl;
        truth = Label::generate(features, featureNames, ALL_FILE, QUALITY_FILE);
    }
    int recognized = Features::recognize(truth);

    
    vector<Pattern> truth_tag;
    if (LABEL_FILE != "") {
        cerr << "=== Load Existing Labels ===" << endl;
        truth_tag = Label::loadLabels(LABEL_FILE);
    } else {
        // generate labels
        cerr << "=== Generate POS tag Labels ===" << endl;
        truth_tag = Label::generate_tag(features_tag, QUALITY_FILE_TAG);
    }
    int recognized_tag = Features::recognize_tag(truth_tag);

    if (true) {
        //FILE* out = tryOpen("tmp/quality_phrases.txt", "w");
        fstream fout;
        fout.open("tmp/postags_label.txt", fstream::out);

        //cout << "dictionary size" << dict.size() << endl;

        for (int iter = 0; iter < truth_tag.size(); ++ iter) {
            //cout << "Iter size" << truth_tag[iter].tokens.size() << endl;
            for (int j = 0; j < truth_tag[iter].tokens.size(); ++ j) {
                fout<<posid2Tag.find(truth_tag[iter].tokens[j])->second;
                if (j + 1 == truth_tag[iter].tokens.size())
                    fout<<endl;
                else
                    fout<<" ";
                //fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
            }
        }
        fout.close();
    }
    
    

    //return -1;

    if (INTERMEDIATE) {
        FILE* out = tryOpen("tmp/generated_label.txt", "w");
        for (Pattern pattern : truth) {
            for (int j = 0; j < pattern.tokens.size(); ++ j) {
                fprintf(out, "%d%c", pattern.tokens[j], j + 1 == pattern.tokens.size() ? '\n' : ' ');
            }
        }
        fclose(out);

        out = tryOpen("tmp/features_for_labels.tsv", "w");
        for (Pattern pattern : truth) {
            int i = FrequentPatternMining::pattern2id[pattern.hashValue];
            if (features[i].size() > 0) {
                for (int j = 0; j < features[i].size(); ++ j) {
                    fprintf(out, "%.10f%c", features[i][j], j + 1 == features[i].size() ? '\n' : '\t');
                }
            }
        }
        fclose(out);
    }

    // SegPhrase, +, ++, +++, ...
    Parser remine;
    if (ENABLE_POS_TAGGING) {
        Segmentation::initializePosTags(Documents::posTag2id.size());
    }
    for (int iteration = 0; iteration < ITERATIONS; ++ iteration) {
        fprintf(stderr, "Feature Matrix = %d X %d\n", features.size(), features.back().size());
        predictQuality(patterns, features, featureNames);
        cerr << "prediction done" << endl;
        if (POSTAG_SCORE == 1){
            predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, patterns, pattern2id_tag);
            cerr<<"predict in the loop"<<endl;
        }

        // check the quality
        if (INTERMEDIATE) {
            char filename[256];
            sprintf(filename, "tmp/quality_patterns_iter_%d.txt", iteration);
            FILE* out = tryOpen(filename, "w");
            vector<pair<double, int>> order;
            for (int i = 0; i < patterns.size(); ++ i) {
                if (patterns[i].size() > 1 && patterns[i].currentFreq > 0) {
                    order.push_back(make_pair(patterns[i].quality, i));
                }
            }
            sort(order.rbegin(), order.rend());
            for (int iter = 0; iter < order.size(); ++ iter) {
                int i = order[iter].second;
                fprintf(out, "%.10f\t", patterns[i].quality);
                for (int j = 0; j < patterns[i].tokens.size(); ++ j) {
                    fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
                }
            }
        }

        constructTrie(); // update the current frequent enough patterns
        if (!ENABLE_POS_TAGGING) {
            cerr << "[Length Penalty Mode]" << endl;
            double penalty = EPS;
            if (true) {
                // Binary Search for Length Penalty
                double lower = EPS, upper = 200;
                for (int _ = 0; _ < 10; ++ _) {
                    penalty = (lower + upper) / 2;
                    //cerr << "iteration " << _ << ": " << penalty << endl;
                    Segmentation segmentation(penalty);
                    segmentation.train(Documents::sentences);
                    double wrong = 0, total = 0;
                    # pragma omp parallel for reduction (+:total,wrong)
                    for (int i = 0; i < truth.size(); ++ i) {
                        if (truth[i].label == 1) {
                            ++ total;
                            vector<double> f;
                            vector<int> pre;
                            segmentation.viterbi(truth[i].tokens, f, pre);
                            wrong += pre[truth[i].tokens.size()] != 0;
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
            segmentation.train(Documents::sentences);
            // Recompute & Augment Features
            Features::augment(features, segmentation, featureNames);
        } else {
            cerr << "[POS Tags Mode]" << endl;
            if (true) {
                Segmentation segmentation(ENABLE_POS_TAGGING);
                double last = 1e100;
                for (int inner = 0; inner < 10; ++ inner) {
                    double energy = segmentation.adjustPOS(Documents::sentences, MIN_SUP);
                    if (fabs(energy - last) / fabs(last) < EPS) {
                        break;
                    }
                    last = energy;
                }
            }

            if (INTERMEDIATE) {
                char filename[256];
                sprintf(filename, "tmp/pos_tags_iter_%d.txt", iteration);
                FILE* out = tryOpen(filename, "w");
                for (int i = 0; i < Documents::posTag.size(); ++ i) {
                    fprintf(out, "\t%s", Documents::posTag[i].c_str());
                }
                fprintf(out, "\n");
                for (int i = 0; i < Documents::posTag.size(); ++ i) {
                    fprintf(out, "%s", Documents::posTag[i].c_str());
                    for (int j = 0; j < Documents::posTag.size(); ++ j) {
                        fprintf(out, "\t%.10f", Segmentation::connect[i][j]);
                    }
                    fprintf(out, "\n");
                }
                fclose(out);
            }

            Segmentation segmentation(ENABLE_POS_TAGGING);
            segmentation.trainPOS(Documents::sentences, MIN_SUP);
            // Recompute & Augment Features
            
            Features::augment(features, segmentation, featureNames);
            //Features::augment_tag(features_tag, segmentation, featureNames_tag);

            if (iteration==ITERATIONS-1){
                cerr<<"Initialize the probability!"<<endl;
                remine.initialize(segmentation.getProb(),segmentation.getSize());
            }
        }

        // check the quality
        if (INTERMEDIATE) {
            char filename[256];
            sprintf(filename, "tmp/frequent_quality_patterns_iter_%d.txt", iteration);
            FILE* out = tryOpen(filename, "w");
            vector<pair<double, int>> order;
            for (int i = 0; i < patterns.size(); ++ i) {
                if (patterns[i].size() > 1 && patterns[i].currentFreq > 0) {
                    order.push_back(make_pair(patterns[i].quality, i));
                }
            }
            sort(order.rbegin(), order.rend());
            for (int iter = 0; iter < order.size(); ++ iter) {
                int i = order[iter].second;
                fprintf(out, "%.10f\t", patterns[i].quality);
                for (int j = 0; j < patterns[i].tokens.size(); ++ j) {
                    fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
                }
            }
        }

        if (INTERMEDIATE) {
            char filename[256];
            sprintf(filename, "tmp/augmented_features_for_labels_%d.tsv", iteration);
            FILE* out = tryOpen(filename, "w");
            for (Pattern pattern : truth) {
                int i = FrequentPatternMining::pattern2id[pattern.hashValue];
                if (features[i].size() > 0) {
                    for (int j = 0; j < features[i].size(); ++ j) {
                        fprintf(out, "%.10f%c", features[i][j], j + 1 == features[i].size() ? '\n' : '\t');
                    }
                }
            }
            fclose(out);
        }
    }

    vector<int> selected = Label::generate_samples(QUALITY_FILE_TAG);
    if (POSTAG_SCORE == 2){    
        predictPosTagQuality(patterns_tag, features_tag, featureNames_tag, selected);
    }
    
    //need to output positive and negative scores


    if (true) {
        //FILE* out = tryOpen("tmp/quality_phrases.txt", "w");
        fstream fout;
        fout.open("tmp/distribution.txt", fstream::out);

        //cout << "dictionary size" << dict.size() << endl;

        for (int i = 0; i < selected.size(); ++ i) {
            int iter = selected[i];
            //cout << "Iter size" << truth_tag[iter].tokens.size() << endl;
            //fout<<patterns_tag[iter].quality<<"\t";
            fout<<patterns_tag[iter].quality<<endl;
            /*
            for (int j = 0; j < patterns_tag[iter].tokens.size(); ++ j) {
                fout<<posid2Tag.find(patterns_tag[iter].tokens[j])->second;
                if (j + 1 == patterns_tag[iter].tokens.size())
                    fout<<endl;
                else
                    fout<<" ";
                //fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
            }
            */
        }
        fout.close();
    }

    unordered_set<int> dict;
    if (true) {
        //FILE* out = tryOpen("tmp/quality_phrases.txt", "w");
        fstream fout;
        fout.open("tmp/quality_postags.txt", fstream::out);
        vector<pair<double, int>> order;
        for (int i = 0; i < patterns.size(); ++ i) {
            //change this line
            if (patterns[i].size() > 1 && patterns[i].currentFreq > 0) {
                order.push_back(make_pair(patterns[i].quality, i));
                if (patterns[i].quality > 0.1)
                    dict.insert(i);
            }
        }

        //cout << "dictionary size" << dict.size() << endl;

        sort(order.rbegin(), order.rend());
        for (int iter = 0; iter < order.size(); ++ iter) {
            int i = order[iter].second;
            //fprintf(out, "%.10f\t", patterns[i].quality);
            fout<<patterns[i].quality<<"\t";
            /*if (patterns[i].tokens.size()==patterns[i].postags.size())
                if (pattern2id_tag.count(patterns[i].getHash()))
                    cout<<"POS tag pattern exists"<<endl;
                else
                    cout<<"POS tag pattern doesn't exist"<<endl;*/
            //cout<<patterns[i].tokens[0]<<" "<<patterns[i].postags.size()<<endl;
            for (int j = 0; j < patterns[i].tokens.size(); ++ j) {
                fout<<posid2Tag.find(patterns[i].postags[j])->second;
                if (j + 1 == patterns[i].tokens.size())
                    fout<<endl;
                else
                    fout<<" ";
                //fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
            }
        }
        //fclose(out);
        fout.close();
    }

    
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
    

    return 0;
}