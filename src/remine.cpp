/*
Use dumped patterns as initialization
Tune the segmentation model under RM mode
*/

#include "utils/config.h"
#include "utils/parameters.h"
#include "utils/remine_flags.h"
#include "utils/utils.h"
#include "frequent_pattern_mining/frequent_pattern_mining.h"
#include "data/documents.h"
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
        quality &= trie[u].id >= 0 && (MODE == 0 || trie[u].indicator == "RP") && (
                    patterns[trie[u].id].size() > 1 && patterns[trie[u].id].quality >= SEGMENT_MULTI_WORD_QUALITY_THRESHOLD ||
                    patterns[trie[u].id].size() == 1 && patterns[trie[u].id].quality >= SEGMENT_SINGLE_WORD_QUALITY_THRESHOLD
                   );

        if (quality) {
            if (MODE == 0) {
                ret.push_back("</"+trie[u].indicator+">");
            }
            else ret.push_back(",");
        }

        if (MODE == 0 || quality) {
            for (int k = i - 1; k >= j; -- k) {
                ostringstream sout;
                sout << tokens[k];  
                //sout << tags[k]; 
                //ret.push_back(Documents::posid2Tag[int(tags[k])]);
                ret.push_back(sout.str());
            }
        }
        
        if (quality && MODE == 0) {
            //if (RELATION_MODE && patterns[i].indicator == "RELATION" || !RELATION_MODE && patterns[i].indicator == "ENTITY")
            ret.push_back("<"+trie[u].indicator+">");
        }

        i = j;
    }

    reverse(ret.begin(), ret.end());
    for (int i = 0; i < ret.size(); ++ i) {
        fprintf(out, "%s%c", ret[i].c_str(), ' ');
    }
    if (MODE == 0) {
        fprintf(out, "\n");
    }
}

inline bool byQuality(const Pattern& a, const Pattern& b)
{
    return a.quality > b.quality + EPS || fabs(a.quality - b.quality) < EPS && a.currentFreq > b.currentFreq;
}

int main(int argc, char* argv[])
{
    parseReMineFlags(argc, argv);

    sscanf(argv[1], "%d", &NTHREADS);
    omp_set_num_threads(NTHREADS);

    Dump::loadSegmentationModel(SEGMENTATION_MODEL_REMINE);

    sort(patterns.begin(), patterns.end(), byQuality);

    constructTrie(); // update the current frequent enough patterns

    Segmentation* segmenter;
    segmenter = new Segmentation(ENABLE_POS_TAGGING, MODE > 0);

    char currentDep[100];
    char currentTag[100];

    FILE* in = tryOpen(TEXT_TO_SEG_REMINE, "r");
    FILE* posIn = tryOpen(TEXT_TO_SEG_POS_TAGS_REMINE, "r");
    FILE* depIn = tryOpen(TEXT_TO_SEG_DEPS_REMINE, "r");
    FILE* emIn = NULL;

    if (MODE == 1) {
        emIn = tryOpen(TEST_EMS_REMINE, "r");
    }

    FILE* out = tryOpen("tmp_remine/remine_tokenized_segmented_sentences.txt", "w");

    int docCount = 0;
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
                myAssert(fscanf(depIn, "%s", currentDep) == 1, "DEP file doesn't have enough DEP tags");

                if (!Documents::posTag2id.count(currentTag)) {
                    posTagId = -1; // unknown tag
                } else {
                    posTagId = Documents::posTag2id[currentTag];
                }
            }

            // get token
            bool flag = true;
            TOKEN_ID_TYPE token = 0;
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
        if (tokens.size() > 0) {
            assert(tokens.size() == deps.size());
            assert(tokens.size() == tags.size());
            ++ docCount;
            if (MODE == 1 && getLine(emIn)) {
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
                
                tmp = GenPath::genSepath(deps, tags, depTypes, ems);
                vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> rm_deps;
                vector<TOKEN_ID_TYPE> rm_tokens;
                for (auto _ = tmp.begin(); _ != tmp.end(); ++_) {
                    const auto& it = _->second;
                    fprintf(out, "%d\t", docCount);
                    for (int i = ems[it.first].first; i < ems[it.first].second; ++ i) {
                        fprintf(out, "%d%s", tokens[i], i + 1 == ems[it.first].second ? "| " : " ");
                    }
                    for (const auto& __ : it.second) {
                        rm_deps.push_back(deps[__ - 1]);
                        rm_tokens.push_back(tokens[__ - 1]);
                    }
                    process(rm_tokens, rm_deps, tags, *segmenter, out);
                    fprintf(out, "| ");
                    for (int i = ems[_->first].first; i < ems[_->first].second; ++ i) {
                        fprintf(out, "%d%c", tokens[i], i + 1 == ems[_->first].second ? '\n' : ' ');
                    }
                    rm_deps.clear();
                    rm_tokens.clear();
                // cout << endl;
                }
            }
            else if (MODE == 0) process(tokens, deps, tags, *segmenter, out);
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
