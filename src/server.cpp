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
#include <deque>
typedef std::vector<string>::iterator vec_iter;

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
    Segmentation* segmenter;
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
        string st = x["key"].s();
        std::ostringstream os;
        os << st;

        return crow::response{os.str()};

    });

    app.port(10086).run();
}
