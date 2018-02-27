#ifndef __DUMP_H__
#define __DUMP_H__

#include "../utils/parameters.h"
#include "../utils/commandline_flags.h"
#include "../utils/utils.h"
#include "../frequent_pattern_mining/frequent_pattern_mining.h"
#include "../data/documents.h"
#include "../classification/feature_extraction.h"
#include "../classification/label_generation.h"
#include "../classification/predict_quality.h"
#include "../model_training/segmentation.h"

namespace Dump
{

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;
using FrequentPatternMining::pattern2id;

void normalizePatterns(vector<pair<double, int>> &order, const int cutff){
    sort(order.rbegin(), order.rend());
    double MIN=order[cutff].first;
    double gap=order[0].first-MIN;
    cerr << MIN << '\t' << gap <<endl;
    for (int i=0; i< cutff;++i)
        patterns[order[i].second].quality=(order[i].first-MIN)/gap;
}
void loadSegmentationModel(const string& filename)
{
    FILE* in = tryOpen(filename, "rb");
    bool flag;
    Binary::read(in, flag);
    myAssert(ENABLE_POS_TAGGING == flag, "Model and configuration mismatch! whether ENABLE_POS_TAGGING?");
    Binary::read(in, Segmentation::penalty);


    // quality phrases & unigrams
    size_t cnt = 0;
    Binary::read(in, cnt);
    patterns.resize(cnt);
    cerr << "# of patterns to load = " << cnt << endl;
    for (int i = 0; i < cnt; ++ i) {
        patterns[i].load(in);
        //cerr<<patterns[i].postagquality<<endl;
    }
    // cerr << "pattern loaded" << endl;

    // Binary::read(in, Documents::totalWordTokens);

    Binary::read(in, cnt);
    FrequentPatternMining::id2ends.resize(cnt);
    Binary::read(in, cnt);
    // cerr << "pattern2id " << cnt <<endl;
    for (int i = 0; i < cnt; ++i) {
        size_t key;
        Binary::read(in, key);
        Binary::read(in, pattern2id[key]);
    }

    // POS Tag mapping
    Binary::read(in, cnt);
    Documents::posTag.resize(cnt);
    for (int i = 0; i < Documents::posTag.size(); ++ i) {
        Binary::read(in, Documents::posTag[i]);
        Documents::posTag2id[Documents::posTag[i]] = i;
        Documents::posid2Tag[i]=Documents::posTag[i];
    }
    cerr << "pos tags loaded" << endl;

    // POS Tag Transition
    Binary::read(in, cnt);
    Segmentation::connect.resize(cnt);
    for (int i = 0; i < Segmentation::connect.size(); ++ i) {
        Segmentation::connect[i].resize(cnt);
        for (int j = 0; j < Segmentation::connect[i].size(); ++ j) {
            Binary::read(in, Segmentation::connect[i][j]);
            //if (Segmentation::connect[i][j] > 0)
            //    cerr << posid2Tag[i] << "+" << posid2Tag[j] << " = " << Segmentation::connect[i][j] << endl;
        }
    }

    // cerr << "POS tag transition loaded" << endl;

    Binary::read(in, cnt);
    Segmentation::tree_map.clear();
    Segmentation::deps_prob.resize(cnt);
    for (int i = 0; i < cnt; ++ i) {
        string key;
        Binary::read(in, key);
        Binary::read(in, Segmentation::tree_map[key]);
        Binary::read(in, Segmentation::deps_prob[Segmentation::tree_map[key]]);
        // Segmentation::deps_prob[Segmentation::tree_map[key]] = 0;
    }
    // cerr << "Tree Maps transition loaded" << endl;

    fclose(in);
}

void dumpSegmentationModel(const string& filename)
{
    FILE* out = tryOpen(filename, "wb");
    Binary::write(out, ENABLE_POS_TAGGING);
    Binary::write(out, Segmentation::penalty);

    // quality phrases & unigrams
    size_t cnt = 0;
    for (int i = 0; i < patterns.size(); ++ i) {
        // if (patterns[i].size() > 1 || patterns[i].size() == 1 && patterns[i].currentFreq > 0 && unigrams[patterns[i].tokens[0]] >= MIN_SUP ) {
            /*if (RELATION_MODE && patterns[i].indicator == "entity" || !RELATION_MODE && patterns[i].indicator == "RELATION") {
                continue;
            }*/
        ++ cnt;
        // }///////
        
    }
    Binary::write(out, cnt);
    cerr << "# of patterns dumped = " << cnt << endl;
    for (int i = 0; i < patterns.size(); ++ i) {
        //if (patterns[i].size() > 1 || patterns[i].size() == 1 && patterns[i].currentFreq > 0 && unigrams[patterns[i].tokens[0]] >= MIN_SUP ) {
            /*if (RELATION_MODE && patterns[i].indicator == "ENTITY" || !RELATION_MODE && patterns[i].indicator == "RELATION") {
                continue;
            }*/
        patterns[i].dump(out);
        //}
    }

    // Binary::write(out, Documents::totalWordTokens);

    // Pattern Id To Tag

    cerr << "# frequent pattern" << id2ends.size() << endl;
    Binary::write(out, FrequentPatternMining::id2ends.size());

    cerr << "# pattern2id" << pattern2id.size() << endl;
    Binary::write(out, pattern2id.size());
    for (const auto& kv : pattern2id) {
        Binary::write(out, kv.first);
        Binary::write(out, kv.second);
    }

    // POS Tag mapping
    Binary::write(out, Documents::posTag.size());
    for (int i = 0; i < Documents::posTag.size(); ++ i) {
        Binary::write(out, Documents::posTag[i]);
    }

    // POS Tag Transition
    Binary::write(out, Segmentation::connect.size());
    for (int i = 0; i < Segmentation::connect.size(); ++ i) {
        for (int j = 0; j < Segmentation::connect[i].size(); ++ j) {
            Binary::write(out, Segmentation::connect[i][j]);
        }
    }

    // 
    Binary::write(out, Segmentation::tree_map.size());
    cerr << "# of tree_maps dumped = " << Segmentation::tree_map.size() << endl;
    for (const auto& kv : Segmentation::tree_map) {
        Binary::write(out, kv.first);
        Binary::write(out, kv.second);
        Binary::write(out, Segmentation::deps_prob[kv.second]);
    }

    fclose(out);
}

void dumpPOSTransition(const string& filename)
{
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

void dumpFeatures(const string& filename, const vector<vector<double>>& features) {
    FILE* out = tryOpen(filename, "wb");
    for (Pattern pattern: patterns) {
        if (pattern.size() > 1) {
            int i = pattern2id[pattern.hashValue];
            if (features[i].size() > 0) {
                for (int j = 0; j < features[i].size(); ++ j) {
                    Binary::write(out, features[i][j]);
                }
            }
        }
    }
    fclose(out);
}

void dumpFeaturesUnigram(const string& filename, const vector<vector<double>>& features) {
    FILE* out = tryOpen(filename, "wb");
    for (Pattern pattern: patterns) {
        if (pattern.size() == 1) {
            int i = pattern2id[pattern.hashValue];
            if (features[i].size() > 0) {
                for (int j = 0; j < features[i].size(); ++ j) {
                    Binary::write(out, features[i][j]);
                }
            }
        }
    }
    fclose(out);
}

vector<vector<double>> loadFeatures(const string& filename, const vector<string>& featureNames) {
    FILE* in = tryOpen(filename, "rb");
    // cerr << "pattern size: " << patterns.size() << endl;
    vector<vector<double>> features(patterns.size(), vector<double>());
    for (Pattern pattern: patterns) {
        if (pattern.size() > 1) {
            int i = pattern2id[pattern.hashValue];
            assert(i < patterns.size());
            //if (features[i].size() > 0) {
            for (int j = 0; j < featureNames.size(); ++ j) {
                features[i].push_back(0);
                Binary::read(in, features[i][j]);
            }
            features[i].shrink_to_fit();
            //}
        }
    }
    features.shrink_to_fit();
    cerr << "here" << endl;
    return features;
}

vector<vector<double>> loadFeaturesUnigram(const string& filename, const vector<string>& featureNames) {
    FILE* in = tryOpen(filename, "rb");
    cerr << "here" << endl;
    vector<vector<double>> features(patterns.size(), vector<double>());
    for (Pattern pattern: patterns) {
        if (pattern.size() == 1) {
            int i = pattern2id[pattern.hashValue];
            //if (features[i].size() > 0) {
            for (int j = 0; j < featureNames.size(); ++ j) {
                features[i].push_back(0);
                Binary::read(in, features[i][j]);
            }
            features[i].shrink_to_fit();
            //}
        }
    }
    features.shrink_to_fit();
    return features;
}

/*
void dumpFeatures(const string& filename, const vector<vector<double>>& features, const vector<Pattern>& truth)
{
    FILE* out = tryOpen(filename, "w");
    for (Pattern pattern : truth) {
        int i = pattern2id[pattern.hashValue];
        if (features[i].size() > 0) {
            for (int j = 0; j < features[i].size(); ++ j) {
                fprintf(out, "%.10f%c", features[i][j], j + 1 == features[i].size() ? '\n' : '\t');
            }
        }
    }
    fclose(out);
}
*/

void dumpLabels(const string& filename, const vector<Pattern>& truth)
{
    FILE* out = tryOpen(filename, "w");
    for (Pattern pattern : truth) {
        for (int j = 0; j < pattern.tokens.size(); ++ j) {
            fprintf(out, "%d%c", pattern.tokens[j], j + 1 == pattern.tokens.size() ? '\n' : ' ');
        }
    }
    fclose(out);
}

template<class T>
void dumpRankingList(const string& filename, vector<pair<T, int>> &order)
{
    FILE* out = tryOpen(filename, "w");
    sort(order.rbegin(), order.rend());
    for (int iter = 0; iter < order.size(); ++ iter) {
        int i = order[iter].second;
        fprintf(out, "%.10f\t", patterns[i].quality);
        fprintf(out, "%s\t", patterns[i].indicator.c_str());
        for (int j = 0; j < patterns[i].tokens.size(); ++ j) {
            fprintf(out, "%d%c", patterns[i].tokens[j], j + 1 == patterns[i].tokens.size() ? '\n' : ' ');
            //fprintf(out, "%d%c", patterns[i].postags[j], j + 1 == patterns[i].postags.size() ? '\n' : ' ');
        }
    }
    fclose(out);
}


void dumpResults(const string& prefix)
{
    vector<pair<double, int>> order;
    //cerr<<"Checkpoint"<<endl;
    for (int i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size() > 1 && patterns[i].currentFreq > 0 && patterns[i].indicator[0] == 'E') {
            order.push_back(make_pair(patterns[i].quality, i));
        }
    }
    //cerr<<"Checkpoint"<<endl;
    dumpRankingList(prefix + "_multi-entities.txt", order);

    order.clear();
    for (int i = 0; i < patterns.size(); ++ i) {
        //now different from view of dumped model or running model
        //if (patterns[i].size() == 1 && patterns[i].currentFreq > 0 && unigrams[patterns[i].tokens[0]] >= MIN_SUP) {
        if (patterns[i].size() > 1 && patterns[i].currentFreq > 0 && patterns[i].indicator[0] == 'B') {
        // if (patterns[i].size() == 1 && patterns[i].currentFreq > 0) {
            order.push_back(make_pair(patterns[i].quality, i));
        }
    }
    //cerr<<"here"<<endl;
    dumpRankingList(prefix + "_multi-backgrounds.txt", order);

    order.clear();
    for (int i = 0; i < patterns.size(); ++ i) {
        //now different from view of dumped model or running model
        if (patterns[i].size() == 1 && patterns[i].currentFreq > 0 && unigrams[patterns[i].tokens[0]] >= MIN_SUP) {
        // if (patterns[i].size() > 1 && patterns[i].currentFreq > 0 && patterns[i].indicator[0] == 'B') {
        // if (patterns[i].size() == 1 && patterns[i].currentFreq > 0) {
            order.push_back(make_pair(patterns[i].quality, i));
        }
    }
    //cerr<<"here"<<endl;
    dumpRankingList(prefix + "_multi-unigrams.txt", order);

    order.clear();
    for (int i = 0; i < patterns.size(); ++ i) {
        //if (patterns[i].size() > 1 && patterns[i].currentFreq > 0 || patterns[i].size() == 1 && patterns[i].currentFreq > 0 && unigrams[patterns[i].tokens[0]] >= MIN_SUP) {
        if (patterns[i].size() > 1 && patterns[i].currentFreq > 0 && patterns[i].indicator[0] == 'R') {
            order.push_back(make_pair(patterns[i].quality, i));
        }
    }
    dumpRankingList(prefix + "_multi-relations.txt", order);
}

};

#endif
