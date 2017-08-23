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

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;
using Documents::tree_map;

vector<double> f;
vector<int> pre;

void process(const vector<TOTAL_TOKENS_TYPE>& tokens, const vector<TOTAL_TOKENS_TYPE>& deps, Segmentation& segmenter, FILE* out)
{
    if (ENABLE_POS_TAGGING) {
        segmenter.viterbi(tokens, deps, f, pre);
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
        quality &= trie[u].id >= 0; // && trie[u].id < SEGMENT_QUALITY_TOP_K;


        if (quality) {
            ret.push_back("</"+trie[u].indicator+">");
            //ret.push_back(to_string(patterns[trie[u].id].quality));
            //cerr<<patterns[trie[u].id].tokens[0]<<" "<<tokens[j]<<endl;
            //ret.push_back(to_string(patterns[trie[u].id].postagquality));
        }

        if (true) {
            for (int k = i - 1; k >= j; -- k) {
                ostringstream sout;
                sout << tokens[k];  
                //sout << tags[k]; 
                //ret.push_back(Documents::posid2Tag[int(tags[k])]);
                ret.push_back(sout.str());
            }
        }
        
        if (quality) {
            ret.push_back("<"+trie[u].indicator+">");
        }

        i = j;
    }

    reverse(ret.begin(), ret.end());
    for (int i = 0; i < ret.size(); ++ i) {
        fprintf(out, "%s%c", ret[i].c_str(), i + 1 == ret.size() ? '\n' : ' ');
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
    /*if (ENABLE_POS_TAGGING) {
        segmenter = new Segmentation(ENABLE_POS_TAGGING);
        Segmentation::getDisconnect();
        Segmentation::logPosTags();
    } else {*/
    segmenter = new Segmentation(Segmentation::penalty);

    char currentDep[100];

    FILE* in = tryOpen(TEXT_TO_SEG_FILE, "r");
    // FILE* posIn = tryOpen(TEXT_TO_SEG_POS_TAGS_FILE, "r");
    FILE* depIn = NULL;

    if (ENABLE_POS_TAGGING) {
        depIn = tryOpen(TEXT_TO_SEG_DEPS_FILE, "r");
    }

    FILE* out = tryOpen("tmp_remine/tokenized_segmented_sentences.txt", "w");


    while (getLine(in)) {
        stringstream sin(line);
        vector<TOTAL_TOKENS_TYPE> tokens;
        vector<TOTAL_TOKENS_TYPE> deps;

        string lastPunc = "";
        for (string temp; sin >> temp;) {
            // get pos tag
            //POS_ID_TYPE posTagId = -1;
            if (ENABLE_POS_TAGGING) {
                myAssert(fscanf(depIn, "%s", currentDep) == 1, "POS file doesn't have enough POS tags");

                //posTagId = currentTag;
            }

            // get token
            bool flag = true;
            TOKEN_ID_TYPE token = 0;
            for (int i = 0; i < temp.size() && flag; ++ i) {
                flag &= /*temp[i] != '.' || */isdigit(temp[i]) || i == 0 && temp.size() > 1 && temp[0] == '-';
            }
            stringstream sin(temp);
            sin >> token;

            if (!flag) {
                string punc = temp;
                if (Documents::separatePunc.count(punc)) {
                    process(tokens, deps, *segmenter, out);
                    tokens.clear();
                    deps.clear();
                }
            } else {
                tokens.push_back(token);
                if (ENABLE_POS_TAGGING) {
                    deps.push_back(atoi(currentDep));
                }
            }
        }
        if (tokens.size() > 0) {
            process(tokens, deps, *segmenter, out);
        }
    }
    fclose(in);
    if (ENABLE_POS_TAGGING) {
        for (const auto& m : tree_map) {
            cerr << m.first << " " << m.second <<endl;
        }
        fclose(depIn);
    }
    fclose(out);

    return 0;
}
