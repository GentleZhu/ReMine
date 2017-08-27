#ifndef __PREDICT_QUALITY_H__
#define __PREDICT_QUALITY_H__

#include "../frequent_pattern_mining/frequent_pattern_mining.h"
using FrequentPatternMining::Pattern;

#include "random_forest.h"
using namespace RandomForestRelated;

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

    solver->train(trainX, trainY, 1000, 1, 100, featureNames);

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
            patterns[i].quality = solver->estimate(features[i]);
        }
	}
	fprintf(stderr, "Prediction done.\n");
}


void combineScore(vector<Pattern> &patterns, vector<Pattern> &patterns_tag, unordered_map<ULL, PATTERN_ID_TYPE> pattern2id){

    int err_cnt=0;

    fprintf(stderr, "Start Combine Score\n");
    for (int i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size()>1){
            if (patterns[i].tokens.size()==patterns[i].postags.size() && pattern2id.count(patterns[i].getHash())){
                int index=pattern2id[patterns[i].getHash()];
                if (patterns_tag[index].size() > 1) {
                    if (patterns_tag[index].quality > patterns_tag[index].qualityB){
                        
                        patterns[i].quality = 2/(1/patterns[i].quality+1/patterns_tag[index].quality);
                        patterns[i].qualityB = patterns_tag[index].quality;
                        if (patterns[i].qualityB > 0.6)
                            patterns[i].indicator="ENTITY";
                        else
                            patterns[i].indicator="None";
                    }
                    else{
                        patterns[i].quality = 2/(1/patterns[i].quality+1/patterns_tag[index].qualityB);
                        patterns[i].qualityB = patterns_tag[index].qualityB;
                        if (patterns[i].qualityB > 0.8)
                            patterns[i].indicator="RELATION";
                        else
                            patterns[i].indicator="None";
                    }
                    //patterns[i].quality*=patterns_tag[index].quality;
                    
                    //patterns[i].quality /= 2;
                    //external_patterns[i].quality = solver->estimate(features[index]);
                    //cout<<solver->estimate(features[index])<<endl;
                }
                //cout<<patterns[index].size()<<endl;
                //cout<<index<<endl;
                //cout<<"Result "<<i<<": "<<solver->estimate(features[index])<<endl;
            }
            else{
                err_cnt+=1;
                //if (patterns[i].size()==1):
                //    patterns[i].indicator="ENTITY";
            }
        }
        //else if (patterns[i].size()==1)
        //    patterns[i].indicator="ENTITY";


                //cerr<<"[ERROR: no matched postags]"<<endl;
    }
    cerr<<"[MISMATCH COUNT]"<<err_cnt<<endl;
    fprintf(stderr, "Combine Score done.\n");
}

void predictPosTagQuality(vector<Pattern> &patterns, vector<vector<double>> &features, vector<string> &featureNames,bool isEntity)
{
    vector<vector<double>> trainX;
    vector<double> trainY;
    int cnt=0;
    int bcnt=0;
    for (int i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size() > 1 && patterns[i].label != FrequentPatternMining::UNKNOWN_LABEL) {
            
            trainX.push_back(features[i]);
            if (patterns[i].label==1 && isEntity==false){
                trainY.push_back(0);
                //++cnt;
            }
            else if (patterns[i].label==2 && isEntity)
                trainY.push_back(0);
            else{
                //if (patterns[i].label==0)
                //    ++bcnt;
                //cout<<(patterns[i].label>0)<<endl;
                trainY.push_back(patterns[i].label>0);
            }
        }
    }
    //cerr<<"labeled input "<<cnt<<" "<<bcnt<<endl;
    //cerr<<trainX.size()<<"<><><>"<<endl;
    cerr<<trainY.size()<<"<><><>"<<endl;
    //cerr<<"feature length is"<<trainX.back().size()<<"<><><>"<<endl;
    
    if (trainX.size() == 0) {
        fprintf(stderr, "[ERROR] no training data found!\n");
        return;
    }

    initialize();
    TASK_TYPE = REGRESSION;
    RandomForest *solver = new RandomForest();
    RANDOM_FEATURES = 4;
    RANDOM_POSITIONS = 4;

    fprintf(stderr, "[POS Tag]Start Classifier Training...\n");
    solver->train(trainX, trainY, 1000, 1, 100, featureNames);

    vector<pair<double, string>> order;
    for (int i = 0; i < featureImportance.size(); ++ i) {
        order.push_back(make_pair(featureImportance[i], featureNames[i]));
    }
    sort(order.rbegin(), order.rend());
    for (int i = 0; i < order.size(); ++ i) {
        cerr << order[i].first << "\t" << order[i].second << endl;
    }


    fprintf(stderr, "[POS Tag]Start Quality Prediction\n");
    for (int i = 0; i < features.size(); ++ i) {
        if (patterns[i].size() > 1) {
            if (isEntity)
                patterns[i].quality = solver->estimate(features[i]);
            else
                patterns[i].qualityB = solver->estimate(features[i]);
        }
    }
    //for (int i = 0; i < features.size(); ++ i) {
    //    if (patterns[i].size() > 1) {
    //        patterns[i].quality = solver->estimate(features[i]);
    //    }
    //}
    //fprintf(stderr, "Prediction done.\n");
}

//predict with selected patterns
void predictPosTagQuality(vector<Pattern> &patterns, vector<vector<double>> &features, vector<string> &featureNames, const vector<int>& selected)
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
    cerr<<trainX.size()<<"<><><>"<<endl;
    cerr<<trainY.size()<<"<><><>"<<endl;
    cerr<<trainX.back().size()<<"<><><>"<<endl;
    
    if (trainX.size() == 0) {
        fprintf(stderr, "[ERROR] no training data found!\n");
        return;
    }

    initialize();
    TASK_TYPE = REGRESSION;
    RandomForest *solver = new RandomForest();
    RANDOM_FEATURES = 14;
    RANDOM_POSITIONS = 8;

    fprintf(stderr, "[POS Tag]Start Classifier Training...\n");
    solver->train(trainX, trainY, 1000, 1, 100, featureNames);

    vector<pair<double, string>> order;
    for (int i = 0; i < featureImportance.size(); ++ i) {
        order.push_back(make_pair(featureImportance[i], featureNames[i]));
    }
    sort(order.rbegin(), order.rend());
    for (int i = 0; i < order.size(); ++ i) {
        cerr << order[i].first << "\t" << order[i].second << endl;
    }

    fprintf(stderr, "[POS Tag]Start Quality Prediction\n");

    for (int i = 0; i < selected.size(); ++ i) {
        int index = selected[i];
        //if (external_patterns[idx].tokens.size()==external_patterns[idx].postags.size())
        if (patterns[index].size() > 1) {
            patterns[index].quality = solver->estimate(features[index]);
            //cout<<solver->estimate(features[index])<<endl;
        }
    }

    //for (int i = 0; i < features.size(); ++ i) {
    //    if (patterns[i].size() > 1) {
    //        patterns[i].quality = solver->estimate(features[i]);
    //    }
    //}
    //fprintf(stderr, "Prediction done.\n");
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

    fprintf(stderr, "[Unigram] Start Quality Prediction\n");
    for (int i = 0; i < features.size(); ++ i) {
        if (patterns[i].size() == 1) {
            patterns[i].quality = solver->estimate(features[i]);
        }
    }
    fprintf(stderr, "[Unigram] Prediction done.\n");
}

#endif
