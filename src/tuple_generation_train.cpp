/*
Use dumped patterns as initialization
Tune the segmentation model under RM mode
*/

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
#include "genSepath.h"

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;

vector<double> f;
vector<int> pre;

void process(const vector<TOTAL_TOKENS_TYPE>& tokens, const vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>>& deps, const vector<TOTAL_TOKENS_TYPE>& tags, Segmentation& segmenter, FILE* out)
{
    if (ENABLE_POS_TAGGING) {
        segmenter.viterbi(tokens, deps, tags, f, pre);
    } else {
        segmenter.viterbi(tokens, f, pre);
    }

    int i = (int)tokens.size();
    // assert(f[i] > -1e80);
    // assert(tokens.size() == deps.size());
    vector<string> ret;
    while (i > 0) {
        int j = pre[i];
        size_t u = 0;
        bool quality = true;
        for (int k = j; k < i; ++ k) {
            if (!trie[u].children.count(tokens[k])) {
                quality = false;
                break;
            }
            u = trie[u].children[tokens[k]];
        }
        quality &= trie[u].id >= 0 && trie[u].indicator == "RP" && (
                    patterns[trie[u].id].size() > 1 && patterns[trie[u].id].quality >= SEGMENT_MULTI_WORD_QUALITY_THRESHOLD ||
                    patterns[trie[u].id].size() == 1 && patterns[trie[u].id].quality >= SEGMENT_SINGLE_WORD_QUALITY_THRESHOLD
                   );

        if (quality) {
            //if (RELATION_MODE && patterns[i].indicator == "RELATION" || !RELATION_MODE && patterns[i].indicator == "ENTITY")
            // ret.push_back("</"+trie[u].indicator+">");
            ret.push_back(",");
            //cerr<<patterns[trie[u].id].tokens[0]<<" "<<tokens[j]<<endl;
            //ret.push_back(to_string(patterns[trie[u].id].postagquality));
        }

        if (quality) {
            for (int k = i - 1; k >= j; -- k) {
                ostringstream sout;
                sout << tokens[k];  
                //sout << tags[k]; 
                //ret.push_back(Documents::posid2Tag[int(tags[k])]);
                ret.push_back(sout.str());
            }
        }
        
        if (quality) {
            //if (RELATION_MODE && patterns[i].indicator == "RELATION" || !RELATION_MODE && patterns[i].indicator == "ENTITY")
            // ret.push_back("<"+trie[u].indicator+">");
        }

        i = j;
    }

    reverse(ret.begin(), ret.end());
    for (int i = 0; i < ret.size(); ++ i) {
        fprintf(out, "%s%c", ret[i].c_str(), ' ');
    }
}

inline bool byQuality(const Pattern& a, const Pattern& b)
{
    return a.quality > b.quality + EPS || fabs(a.quality - b.quality) < EPS && a.currentFreq > b.currentFreq;
}

int main(int argc, char* argv[])
{
    parseCommandFlags(argc, argv);

    sscanf(argv[1], "%d", &NTHREADS);
    omp_set_num_threads(NTHREADS);

    Dump::loadSegmentationModel(SEGMENTATION_MODEL);
    
    //for (int i=0;i<patterns.size();++i)
    //    cerr<<"check:"<<patterns[i].postagquality<<endl;

    //Output ranking List
    //cerr<<"Checkpoint"<<endl;
    //Dump::dumpResults("tmp_ranklist/ori_final_quality");

    //return 0;

    sort(patterns.begin(), patterns.end(), byQuality);
    int unigram_cnt=0,multigram_cnt=0;
    for (int i=0;i<patterns.size();++i){
        if (patterns[i].quality <= 0)
            continue;
        if (patterns[i].size()==1)
            unigram_cnt+=1;
        else
            multigram_cnt+=1;
    }
    cerr << "unigram_cnt:" << unigram_cnt << " multigram_cnt:" << multigram_cnt <<endl;

    //    cerr<<"check:"<<patterns[i].postagquality<<endl;

    constructTrie(); // update the current frequent enough patterns

    Segmentation* segmenter;
    if (ENABLE_POS_TAGGING) {
        segmenter = new Segmentation(ENABLE_POS_TAGGING, true);
        // Segmentation::logPosTags();
    } else {
        segmenter = new Segmentation(Segmentation::penalty);
    }

    char currentDep[100];
    char currentTag[100];

    FILE* in = tryOpen(TEXT_TO_SEG_FILE, "r");
    FILE* posIn = tryOpen(TEXT_TO_SEG_POS_TAGS_FILE, "r");
    FILE* emIn = NULL;
    FILE* depIn = NULL;

    if (ENABLE_POS_TAGGING) {
        depIn = tryOpen(RM_TRAIN_DEPS_FILE, "r");
        emIn = tryOpen(RM_TRAIN_EMS_FILE, "r");
    }

    FILE* out = tryOpen("tmp_remine/rm_tokenized_segmented_sentences.txt", "w");


    while (getLine(in)) {
        stringstream sin(line);
        vector<TOTAL_TOKENS_TYPE> tokens;
        // vector<TOTAL_TOKENS_TYPE> deps;
        vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> deps;
        vector<string> depTypes;
        vector<TOTAL_TOKENS_TYPE> tags;

        string lastPunc = "";
        for (string temp; sin >> temp;) {
            // get pos tag
            POS_ID_TYPE posTagId = -1;
            if (ENABLE_POS_TAGGING) {
                myAssert(fscanf(posIn, "%s", currentTag) == 1, "POS file doesn't have enough POS tags");
                myAssert(fscanf(depIn, "%s", currentDep) == 1, "POS file doesn't have enough POS tags");

                if (!Documents::posTag2id.count(currentTag)) {
                    posTagId = -1; // unknown tag
                } else {
                    posTagId = Documents::posTag2id[currentTag];
                }
            }

            // get token
            bool flag = true;
            TOKEN_ID_TYPE token = 0;
            for (int i = 0; i < temp.size() && flag; ++ i) {
                flag &= /*temp[i] != '.' || */isdigit(temp[i]) || i == 0 && temp.size() > 1 && temp[0] == '-';
            }
            stringstream sin(temp);
            sin >> token;
            tokens.push_back(token);
            if (ENABLE_POS_TAGGING) {
                tags.push_back(posTagId);
                int idx = atoi(strtok (currentDep, "_"));
                int idx_dep = atoi(strtok (NULL, "_"));
                string xxx(strtok(NULL, "_"));
                depTypes.push_back(xxx);
                // deps.push_back(atoi(currentDep));
                deps.push_back(make_pair(idx, idx_dep));
            }
        }
        if (getLine(emIn) && tokens.size() > 0) {

            stringstream sin(line);
            vector<pair<int ,int>> ems;
            unordered_map<int, pair<int, set<TOTAL_TOKENS_TYPE>>> tmp;
            for(string temp; sin >> temp;) {
                vector<string> segs;
                GenPath::split(temp, '_', segs);
                assert(segs.size() == 2);
                ems.push_back(make_pair(stoi(segs[0]), stoi(segs[1])));
            }
            // remember ranges -1
            assert(tokens.size() == deps.size());
            assert(tokens.size() == tags.size());
            // cout << "EM size:" << ems.size() << endl;
            tmp = GenPath::genSepath(deps, tags, depTypes, ems);
            vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> rm_deps;
            vector<TOKEN_ID_TYPE> rm_tokens;
            for (auto _ = tmp.begin(); _ != tmp.end(); ++_) {
                const auto& it = _->second;
                for (int i = ems[it.first].first; i < ems[it.first].second; ++ i) {
                    fprintf(out, "%d%s", tokens[i], i + 1 == ems[it.first].second ? ", " : " ");
                }
                for (const auto& __ : it.second) {
                    rm_deps.push_back(deps[__ - 1]);
                    rm_tokens.push_back(tokens[__ - 1]);
                }
                process(rm_tokens, rm_deps, tags, *segmenter, out);
                for (int i = ems[_->first].first; i < ems[_->first].second; ++ i) {
                    fprintf(out, "%d%c", tokens[i], i + 1 == ems[_->first].second ? '\n' : ' ');
                }
                rm_deps.clear();
                rm_tokens.clear();
                // cout << endl;
            }
            // cout << "here\t"  << tokens.size() << endl;
        }
        // process(tokens, deps, tags, *segmenter, out);
        tokens.clear();
        deps.clear();
        depTypes.clear();
        tags.clear();

        }
        
        // test 
        // break;
    fclose(in);
    fclose(out);

    return 0;
}
