#ifndef __PREDICT_QUALITY_H__
#define __PREDICT_QUALITY_H__

#include "../frequent_pattern_mining/frequent_pattern_mining.h"
using FrequentPatternMining::Pattern;

#include "random_forest.h"
using namespace RandomForestRelated;
#include "../data/documents.h"


void predictQuality(vector<Pattern> &patterns, vector<vector<double>> &features, vector<string> &featureNames)
{
    vector<vector<double>> trainX;
    vector<double> trainY;
    int cnt=0;
    for (int i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size() > 1 && patterns[i].label != FrequentPatternMining::UNKNOWN_LABEL) {
            ++ cnt;
            trainX.push_back(features[i]);
            trainY.push_back(patterns[i].label);
        }
    }

    cerr<<"labeled input "<<cnt<<endl;

    if (trainX.size() == 0) {
        fprintf(stderr, "[ERROR] no training data found!\n");
        return;
    }

    initialize();
    TASK_TYPE = REGRESSION;
    RandomForest *solver = new RandomForest();
    RANDOM_FEATURES = 4;
    RANDOM_POSITIONS = 4;

    fprintf(stderr, "Start Classifier Training...\n");

    cerr << trainX.size() << trainY.size() << endl;

    solver->train(trainX, trainY, 1000, 1, 100, featureNames);

    cerr << "passed" << endl;

    vector<pair<double, string>> order;
    for (int i = 0; i < featureImportance.size(); ++ i) {
        order.push_back(make_pair(featureImportance[i], featureNames[i]));
    }
    sort(order.rbegin(), order.rend());
    for (int i = 0; i < order.size(); ++ i) {
        cerr << order[i].first << "\t" << order[i].second << endl;
    }

    fprintf(stderr, "Start Quality Prediction\n");
    for (int i = 0; i < features.size(); ++ i) {
        if (patterns[i].size() > 1) {
            //patterns[i].quality = LiblinearRelated::predict(features[i]);
            patterns[i].quality = solver->estimate(features[i]) - 1;
            patterns[i].indicator = "ENTITY";
            if (patterns[i].quality < 0) {
                // cerr << patterns[i].quality << endl;
                patterns[i].indicator = "RELATION";
                patterns[i].quality = - patterns[i].quality;
            }
            /*
            if (RELATION_MODE && patterns[i].indicator == "ENTITY") {
                patterns[i].quality = 0;
            }
            if (!RELATION_MODE && patterns[i].indicator == "RELATION") {
                patterns[i].quality = 0;
            }
            */
        }
    }
    fprintf(stderr, "Prediction done.\n");
}

void predictQualityUnigram(vector<Pattern> &patterns, vector<vector<double>> &features, vector<string> &featureNames)
{
    vector<vector<double>> trainX;
    vector<double> trainY;
    for (int i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size() == 1 && patterns[i].label != FrequentPatternMining::UNKNOWN_LABEL) {
            trainX.push_back(features[i]);
            trainY.push_back(patterns[i].label);
        }
    }

    if (trainX.size() == 0) {
        fprintf(stderr, "[ERROR] no training data found!\n");
        return;
    }

    initialize();
    TASK_TYPE = REGRESSION;
    RandomForest *solver = new RandomForest();
    RANDOM_FEATURES = 4;
    RANDOM_POSITIONS = 4;

    fprintf(stderr, "[Unigram] Start Classifier Training...\n");
    solver->train(trainX, trainY, 1000, 1, 100, featureNames);

    vector<pair<double, string>> order;
    for (int i = 0; i < featureImportance.size(); ++ i) {
        order.push_back(make_pair(featureImportance[i], featureNames[i]));
    }
    sort(order.rbegin(), order.rend());
    for (int i = 0; i < order.size(); ++ i) {
        cerr << order[i].first << "\t" << order[i].second << endl;
    }
    set<string> entity_tag = {"NN", "NNS", "NNP", "NNPS", "PRP"};
    fprintf(stderr, "[Unigram] Start Quality Prediction\n");
    for (int i = 0; i < features.size(); ++ i) {
        if (patterns[i].size() == 1) {
            patterns[i].quality = solver->estimate(features[i]) - 1;
            //patterns[i].quality = fabs(solver->estimate(features[i]) - 1);
            
            assert(patterns[i].postags.size() == 1);
            if (entity_tag.count(Documents::posid2Tag[patterns[i].postags[0]]) &&
                patterns[i].quality > 0) {
                patterns[i].indicator = "ENTITY";
            }
            else {
                patterns[i].indicator = "RELATION"; 
            }
            patterns[i].quality = fabs(patterns[i].quality);
        }
    }
    fprintf(stderr, "[Unigram] Prediction done.\n");
}

#endif
