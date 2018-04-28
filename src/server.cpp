/*
Use dumped patterns as initialization
Tune the segmentation model under RM mode
*/
#include "utils/crow_all.h"
#include "utils/config.h"
#include "utils/parameters.h"
#include "utils/remine_flags.h"
#include "utils/utils.h"
#include "frequent_pattern_mining/frequent_pattern_mining.h"
#include "data/documents.h"
#include "model_training/segmentation.h"
#include "data/dump.h"
#include "genSepath.h"
#include <cstring>
#include <stdio.h>
#include <deque>
typedef std::vector<string>::iterator vec_iter;

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;

vector<double> f;
vector<int> pre;
Segmentation* segmenter;

void process(const vector<TOTAL_TOKENS_TYPE>& tokens, const vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>>& deps, const vector<TOTAL_TOKENS_TYPE>& tags, Segmentation& segmenter, std::ostringstream* out)
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
    std::cout<<"process";
    reverse(ret.begin(), ret.end());
    for (int i = 0; i < ret.size(); ++ i) {
        //fprintf(out, "%s%c", ret[i].c_str(), ' ');
        char buf[300];
        sprintf(buf, "%s%c", ret[i].c_str(), ' ');
        string tmp_out = buf;
        //*out += tmp_out;
    }
    if (MODE == 0) {
        //fprintf(out, "\n");
        string tmp_out = "\n";
        //*out += tmp_out;
    }
}

inline bool byQuality(const Pattern& a, const Pattern& b)
{
    return a.quality > b.quality + EPS || fabs(a.quality - b.quality) < EPS && a.currentFreq > b.currentFreq;
}

int main()
{
    //HYPER PARAMETER
    MODE = 1;
    SEGMENTATION_MODEL_REMINE = "pre_train/segmentation.model";
    omp_set_num_threads(NTHREADS);

    // load model
    Dump::loadSegmentationModel(SEGMENTATION_MODEL_REMINE);
    sort(patterns.begin(), patterns.end(), byQuality);
    constructTrie(); // update the current frequent enough patterns
    segmenter = new Segmentation(ENABLE_POS_TAGGING, MODE > 0);


    //WEB API
    crow::SimpleApp app;

    //GET RESULT FROM PYTHON WEB
    CROW_ROUTE(app, "/pass_result")
    .methods("GET"_method)
    ([](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x){
            return crow::response(400);
            }

      char currentDep[100];
//      char currentTag[100];

        //get input from user
        string tokens_text = x["tokens"].s();
        string pos_text = x["pos"].s();
        string dep_text = x["dep"].s();
        string ent_text = x["ent"].s();

//        if (MODE == 1) {
//        emIn = tryOpen(TEST_EMS_REMINE, "r");
//        }

        //FILE* out = tryOpen("tmp_remine/remine_tokenized_segmented_sentences.txt", "w");


        //process strings
        std::istringstream token_sin(tokens_text);
        std::istringstream dep_sin(dep_text);
        std::istringstream pos_sin(pos_text);
        std::istringstream ent_sin(ent_text);
        std::string token_line;
        std::string dep_line;
        std::string pos_line;
        std::string ent_line;

        int docCount = 0;

        while(std::getline(token_sin,token_line)) {
            std::cout<<"begin";
            std::ostringstream out;
            std::cout<<token_line<<'\n';
            stringstream sin(token_line);
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
                    //std::getline(pos_sin,pos_line);
                    //std::getline(dep_sin,dep_line);
                    std::cout<<"check1";
                    myAssert(static_cast<bool>(std::getline(pos_sin,pos_line)), "POS file doesn't have enough POS tags");
                    myAssert(static_cast<bool>(std::getline(dep_sin,dep_line)), "DEP file doesn't have enough DEP tags");

                    if (!Documents::posTag2id.count(pos_line)) {
                        posTagId = -1; // unknown tag
                    } else {
                        posTagId = Documents::posTag2id[pos_line];
                    }
                }

                // get token
                bool flag = true;
                TOKEN_ID_TYPE token = 0;
                stringstream sin(temp);
                sin >> token;
                tokens.push_back(token);
                std::cout<<"check2";
                if (ENABLE_POS_TAGGING) {
                    tags.push_back(posTagId);
                    std::strcpy(currentDep, dep_line.c_str());
                    int idx = atoi(strtok (currentDep, "_"));
                    int idx_dep = atoi(strtok (NULL, "_"));
                    string xxx(strtok(NULL, "_"));
                    depTypes.push_back(xxx);
                    // deps.push_back(atoi(currentDep));
                    deps.push_back(make_pair(idx, idx_dep));
                }
            }
            if (tokens.size() > 0) {
                std::cout<<"check3";
                assert(tokens.size() == deps.size());
                assert(tokens.size() == tags.size());
                ++ docCount;
                if (MODE == 1 && std::getline(ent_sin,ent_line)) {
                    stringstream sin(ent_line);
                    vector<pair<int ,int>> ems;
                    unordered_map<int, pair<int, set<TOTAL_TOKENS_TYPE>>> tmp;
                    for(string temp; sin >> temp;) {
                        vector<string> segs;
                        GenPath::split(temp, '_', segs);
                        assert(segs.size() == 2);
                        ems.push_back(make_pair(stoi(segs[0]), stoi(segs[1])));
                    }
                    // remember ranges -1
                    std::cout<<deps.size()<<"\n";
                    std::cout<<tags.size()<<"\n";
                    std::cout<<depTypes.size()<<"\n";
                    std::cout<<ems.size()<<"\n";

                    tmp = GenPath::genSepath(deps, tags, depTypes, ems);
                    vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> rm_deps;
                    vector<TOKEN_ID_TYPE> rm_tokens;
                    std::cout<<tmp.size()<<"genpath\n";
                    std::cout<<"check4";
                    for (auto _ = tmp.begin(); _ != tmp.end(); ++_) {
                        std::cout<<"check5";
                        const auto& it = _->second;
                        //fprintf(out, "%d\t", docCount);
                        char buf [20];
                        sprintf(buf, "%d\t", docCount);
                        string tmp_out = buf;
                        //std::cout<<tmp_out<<"tmp\n";

                        out<<"adadad";
                        std::cout<<"I pass";
                        //string test_out = out.str();
                        //std::cout<<out;


                        for (int i = ems[it.first].first; i < ems[it.first].second; ++ i) {
                            //fprintf(out, "%d%s", tokens[i], i + 1 == ems[it.first].second ? "| " : " ");
                            char buf [250];

//                            if (tokens[i]%i + 1 == ems[it.first].second) {
//                                f = '| ';
//
//                            }
//                            else{
//                                f = " ";
//                            }
                            sprintf(buf, "%s%s",tokens[i],i + 1 == ems[it.first].second ? "| " : " ");
                            //string tmp_out = buf;
                            //out += tmp_out;
                        }
                        for (const auto& __ : it.second) {
                            rm_deps.push_back(deps[__ - 1]);
                            rm_tokens.push_back(tokens[__ - 1]);
                        }

                        //run process
                        std::cout<<"start process";
                        process(rm_tokens, rm_deps, tags, *segmenter, &out);
                        std::cout<<"finish process";
                        //fprintf(out, "| ");
                        //string tmp_out_second = "| ";
                        //out += tmp_out_second;
                    for (int i = ems[_->first].first; i < ems[_->first].second; ++ i) {
                        //fprintf(out, "%d%c", tokens[i], i + 1 == ems[_->first].second ? '\n' : ' ');
                        char buf[250];
//                            if (tokens[i]%i + 1 == ems[it.first].second) {
//                                f = "\n";
//
//                            }
//                            else{
//                                f = ' ';
//                            }
                        sprintf(buf, "%s%s",tokens[i],i + 1 == ems[it.first].second ? '\n' : ' ');
                        string tmp_out = buf;
                        //out += tmp_out;


                    }
                    rm_deps.clear();
                    rm_tokens.clear();
                // cout << endl;
                }
            }
            else if (MODE == 0) process(tokens, deps, tags, *segmenter, &out);
            // cout << "here\t"  << tokens.size() << endl;
        }


        tokens.clear();
        deps.clear();
        depTypes.clear();
        tags.clear();

        //output
        string s = out.str();
        std::cout<<s;
        }
    //fclose(out);


        return crow::response{'f'};

    });

    app.port(10086).run();
}
