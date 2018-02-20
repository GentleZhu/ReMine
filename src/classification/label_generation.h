#ifndef __LABEL_GENERATION_H__
#define __LABEL_GENERATION_H__

#include "../utils/utils.h"
#include "../data/documents.h"
#include "../frequent_pattern_mining/frequent_pattern_mining.h"
// #include "../clustering/clustering.h"
#include "../classification/predict_quality.h"

using Documents::totalWordTokens;
using Documents::wordTokens;

using FrequentPatternMining::Pattern;
using FrequentPatternMining::patterns;
using FrequentPatternMining::pattern2id;
using FrequentPatternMining::id2ends;

using FrequentPatternMining::patterns_tag;
using FrequentPatternMining::pattern2id_tag;
using Documents::posTag2id;
using Documents::posid2Tag;
using FrequentPatternMining::unigrams;

namespace Label

{
unordered_map<ULL, TOTAL_TOKENS_TYPE> BigramID;


inline vector<Pattern> loadLabels(string filename)
{
    vector<Pattern> ret;
    FILE* in = tryOpen(filename, "r");
    while (getLine(in)) {
        stringstream sin(line);
        bool valid = true;
        Pattern p;
        sin >> p.label;
        for (TOKEN_ID_TYPE s; sin >> s;) {
            p.append(s);
        }
        if (p.size() > 0 && pattern2id.count(p.hashValue)) {
            ret.push_back(p);
        }
    }
    cerr << "# of loaded labels = " << ret.size() << endl;
    return ret;
}

inline unordered_set<ULL> loadPatterns(string filename, int MAX_POSITIVE)
{
    FILE* in = tryOpen(filename, "r");
    // cerr << "DDDDDDEBUG" << pattern2id.size() << endl;
    vector<ULL> positivesUnigrams, positiveMultiwords;
    while (getLine(in)) {
        stringstream sin(line);
        bool valid = true;
        Pattern p;
        for (string s; sin >> s;) {
            bool possibleInt = false;
            for (int i = 0; i < s.size(); ++ i) {
                possibleInt |= isdigit(s[i]);
            }
            if (possibleInt) {
                TOKEN_ID_TYPE x;
                fromString(s, x);
                if (x < 0) {
                    valid = false;
                    break;
                }
                p.append(x);
            }
        }
        if (valid && pattern2id.count(p.hashValue)) {
            if (p.size() > 1) {
                positiveMultiwords.push_back(p.hashValue);
            } else if (p.size() == 1) {
                positivesUnigrams.push_back(p.hashValue);
            }
        }
    }
    fclose(in);

    if (MAX_POSITIVE != -1) {
        sort(positiveMultiwords.begin(), positiveMultiwords.end());
        positiveMultiwords.erase(unique(positiveMultiwords.begin(), positiveMultiwords.end()), positiveMultiwords.end());
        if (MAX_POSITIVE < positiveMultiwords.size()) {
            srand(time(0) ^ 13548689);
            random_shuffle(positiveMultiwords.begin(), positiveMultiwords.end());
            positiveMultiwords.resize(MAX_POSITIVE);
        }
        sort(positivesUnigrams.begin(), positivesUnigrams.end());
        positivesUnigrams.erase(unique(positivesUnigrams.begin(), positivesUnigrams.end()), positivesUnigrams.end());
        if (MAX_POSITIVE < positivesUnigrams.size()) {
            srand(time(0) ^ 13548689);
            random_shuffle(positivesUnigrams.begin(), positivesUnigrams.end());
            positivesUnigrams.resize(MAX_POSITIVE);
        }
    }
    unordered_set<ULL> ret;
    for (ULL value : positiveMultiwords) {
        ret.insert(value);
    }
    cerr << "positiveMultiwords" << ret.size () << endl;
    for (ULL value : positivesUnigrams) {
        ret.insert(value);
    }
    cerr << "positiveTotalwords" << ret.size() << endl;
    return ret;
}

inline vector<PATTERN_ID_TYPE> samplingByLength(vector<PATTERN_ID_TYPE> all, PATTERN_ID_TYPE total, PATTERN_ID_TYPE base = 0)
{
    if (total <= 0 || total >= all.size()) {
        return all;
    }

    vector<PATTERN_ID_TYPE> ret;
    unordered_map<int, vector<PATTERN_ID_TYPE>> groups;
    for (PATTERN_ID_TYPE id : all) {
        groups[patterns[id].size()].push_back(id);
    }
    PATTERN_ID_TYPE accumulated = 0, remain = total - base * groups.size();
    if (remain < 0) {
        remain = total;
        base = 0;
    }

    for (auto& iter : groups) {
        vector<PATTERN_ID_TYPE>& group = iter.second;
        PATTERN_ID_TYPE weight = (PATTERN_ID_TYPE)group.size() - base;
        assert(weight >= 0);

        double percent1 = (double)accumulated / (all.size() - base * groups.size());
        double percent2 = (double)(accumulated + weight) / (all.size() - base * groups.size());

        accumulated += weight;
        assert(accumulated <= all.size() - base * groups.size());

        PATTERN_ID_TYPE limit = (PATTERN_ID_TYPE)(remain * percent2) - (PATTERN_ID_TYPE)(remain * percent1) + base;
        assert(limit >= 0 && limit < group.size());
        random_shuffle(group.begin(), group.end());
        if (limit < group.size()) {
            group.resize(limit);
        }

        for (PATTERN_ID_TYPE id : group) {
            ret.push_back(id);
        }
    }
    return ret;
}

inline vector<Pattern> generateBootstrap(vector<vector<double>> &features, vector<string> &featureNames, vector<PATTERN_ID_TYPE> &positives, vector<PATTERN_ID_TYPE> &negatives)
{
    vector<Pattern> ret;
    srand(19910724);
    // randomly choose the initial labels
    random_shuffle(positives.begin(), positives.end());
    random_shuffle(negatives.begin(), negatives.end());

    PATTERN_ID_TYPE cntPositives = 0;
    for (PATTERN_ID_TYPE i = 0; i < positives.size() && i < MAX_POSITIVE; ++ i) {
        patterns[positives[i]].label = 1;
        ++ cntPositives;
    }
    for (PATTERN_ID_TYPE i = 0; i < negatives.size() && i < cntPositives * NEGATIVE_RATIO; ++ i) {
        patterns[negatives[i]].label = 0;
    }
    predictQuality(patterns, features, featureNames);

    for (PATTERN_ID_TYPE i = 0; i < positives.size() && i < MAX_POSITIVE; ++ i) {
        patterns[positives[i]].label = FrequentPatternMining::UNKNOWN_LABEL;
    }
    for (PATTERN_ID_TYPE i = 0; i < negatives.size() && i < cntPositives * NEGATIVE_RATIO; ++ i) {
        patterns[negatives[i]].label = FrequentPatternMining::UNKNOWN_LABEL;
    }

    vector<PATTERN_ID_TYPE> newPositives, newNegatives;
    for (PATTERN_ID_TYPE i = 0; i < positives.size(); ++ i) {
        if (patterns[positives[i]].quality > 0.1) {
            newPositives.push_back(positives[i]);
        }
    }
    for (PATTERN_ID_TYPE i = 0; i < negatives.size(); ++ i) {
        if (patterns[negatives[i]].quality < 0.9) {
            newNegatives.push_back(negatives[i]);
        }
    }
    random_shuffle(newPositives.begin(), newPositives.end());
    random_shuffle(newNegatives.begin(), newNegatives.end());

    // positives
    if (cntPositives < newPositives.size()) {
        newPositives.resize(cntPositives);
    }
    if (newPositives.size() < newNegatives.size()) {
        newNegatives.resize(newPositives.size());
    }
    for (PATTERN_ID_TYPE id : newPositives) {
        ret.push_back(patterns[id]);
        ret.back().label = 1;
    }
    for (PATTERN_ID_TYPE id : newNegatives) {
        ret.push_back(patterns[id]);
        ret.back().label = 0;
    }

    fprintf(stderr, "selected positives = %d\n", positives.size());
    fprintf(stderr, "selected negatives = %d\n", newNegatives.size());

    return ret;
}

inline vector<Pattern> generateAll(string LABEL_METHOD, string LABEL_FILE, string QUALITY_FILE, string NEGATIVES_FILE)
{
    vector<Pattern> ret;

    if (LABEL_METHOD.find("E") != -1) { // experts
        vector<Pattern> truth;
        cerr << "Loading existing labels..." << endl;
        truth = Label::loadLabels(LABEL_FILE);
        bool needPos = LABEL_METHOD.find("EP") != -1;
        bool needNeg = LABEL_METHOD.find("EN") != -1;
        for (PATTERN_ID_TYPE i = 0; i < truth.size(); ++ i) {
            if (truth[i].label == 1) {
                if (needPos) {
                    ret.push_back(truth[i]);
                }
            } else if (truth[i].label == 0) {
                if (needNeg) {
                    ret.push_back(truth[i]);
                }
            }
        }
    }

    vector<int> negatives;
    if (LABEL_METHOD.find("D") != -1) { // distant training
        bool needPos = LABEL_METHOD.find("DP") != -1;
        bool needNeg = LABEL_METHOD.find("DN") != -1;

        unordered_set<ULL> include = loadPatterns(QUALITY_FILE, MAX_POSITIVE);
        unordered_set<ULL> exclude = loadPatterns(NEGATIVES_FILE, MAX_POSITIVE);

        cerr << "Positive entity label size is" << include.size() << endl;
        cerr << "Negatives entity label size is" << exclude.size() << endl;

        for (PATTERN_ID_TYPE i = 0; i < id2ends.size(); ++ i) {
            if (patterns[i].size() < 1) {
                continue;
            }
            if (include.count(patterns[i].hashValue)) {
                if (needPos) {
                    ret.push_back(patterns[i]);
                    ret.back().label = 1;
                    patterns[i].label = 1;
                }
            } else if (exclude.count(patterns[i].hashValue)) {
                if (needPos) {
                    ret.push_back(patterns[i]);
                    ret.back().label = 2;
                    patterns[i].label = 2;
                }
            } 
            else if (!include.count(patterns[i].hashValue)) {
                if (needNeg) {
                    ret.push_back(patterns[i]);
                    ret.back().label = 0;
                    patterns[i].label = 0;
                    // negatives.push_back(i);
                }
            }
        }

    }

    int cntPositives = 0, cntNegatives = 0;
    for (PATTERN_ID_TYPE i = 0; i < ret.size(); ++ i) {
        if (ret[i].label == 2 || ret[i].label == 1) {
            ++ cntPositives;
        } else if (ret[i].label == 0) {
            ++ cntNegatives;
        } else {
            assert(false); // It should not happen!
        }
    }

    fprintf(stderr, "\tThe size of the positive pool = %d\n", cntPositives);
    fprintf(stderr, "\tThe size of the negative pool = %d\n", cntNegatives);

    return ret;
}

void removeWrongLabels()
{
    int cnt = 0;
    for (Pattern& pattern : patterns) {
        if (pattern.currentFreq == 0 && pattern.label == 1) {
            pattern.label = FrequentPatternMining::UNKNOWN_LABEL;
            ++ cnt;
        }
    }
    //Here is removing wrong labels
}

}

#endif
