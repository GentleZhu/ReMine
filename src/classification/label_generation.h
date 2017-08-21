#ifndef __LABEL_GENERATION_H__
#define __LABEL_GENERATION_H__

#include "../utils/utils.h"
#include "../data/documents.h"
#include "../frequent_pattern_mining/frequent_pattern_mining.h"
#include "../clustering/clustering.h"
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

inline unordered_set<ULL> loadPatterns_tag(string filename)
{
    FILE* in = tryOpen(filename, "r");
    unordered_set<ULL> ret;
    int dict_size=0;
    while (getLine(in)) {
        stringstream sin(line);
        bool valid = true;
        Pattern p;
        string previous="";
        for (string s; sin >> s;) {
            p.append(posTag2id.find(s)->second);
            if (previous.size()>0){
                Pattern tmp;
                tmp.append(posTag2id.find(previous)->second);
                tmp.append(posTag2id.find(s)->second);
                if (pattern2id_tag.count(tmp.hashValue)&&!BigramID.count(tmp.hashValue)){
                    //cerr<<previous<<" "<<s<<" "<<dict_size<<endl;
                    BigramID[tmp.hashValue]=dict_size++;
                }
            }
            previous=s;
            /*bool possibleInt = false;
            for (int i = 0; i < s.size(); ++ i) {
                possibleInt |= isdigit(s[i]);
            }
            if (possibleInt) {
                int x;
                fromString(s, x);
                if (x < 0) {
                    valid = false;
                    break;
                }
                p.append(x);
            }*/
        }
        if (valid && p.size() > 1) {
            ret.insert(p.hashValue);
        }
    }
    cerr<<"dictionary size is "<<dict_size<<" "<<BigramID.size()<<endl;
    fclose(in);
    return ret;
}

inline unordered_set<ULL> loadPatterns(string filename, int MAX_POSITIVE)
{
    FILE* in = tryOpen(filename, "r");
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
    for (ULL value : positivesUnigrams) {
        ret.insert(value);
    }
    return ret;
}

inline vector<int> select(vector<int> candidates, const vector<vector<double>> &features, int n)
{
    if (n > candidates.size()) {
        fprintf(stderr, "[WARNING] labels may not be enough. Only %d\n", candidates.size());
        return candidates;
    }

    if (candidates.size() > 10 * n) {
        random_shuffle(candidates.begin(), candidates.end());
        candidates.resize(10 * n);
    }

    HardClustering *solver = new KMeans(n);
    vector<vector<double>> points;
    for (int i : candidates) {
        points.push_back(features[i]);
    }
    # pragma omp parallel for
    for (int j = 0; j < points[0].size(); ++ j) {
        double sum = 0, sum2 = 0;
        for (int i = 0; i < points.size(); ++ i) {
            sum += points[i][j];
            sum2 += sqr(points[i][j]);
        }
        double avg = sum / points.size();
        double stderror = sqrt(fabs(sum2 / points.size() - avg * avg));
        if (stderror < EPS) {
            cerr << "useless feature " << j << endl;
        }
        for (int i = 0; i < points.size(); ++ i) {
            points[i][j] -= avg;
            if (stderror > EPS) {
                points[i][j] /= stderror;
            }
        }
    }
    vector<int> assignment = solver->clustering(points);

    vector<vector<int>> clusters(n, vector<int>());
    for (int i = 0; i < assignment.size(); ++ i) {
        clusters[assignment[i]].push_back(candidates[i]);
    }
    vector<int> ret;
    for (vector<int> arr : clusters) {
        if (arr.size()) {
            ret.push_back(arr[rand() % arr.size()]);
        }
    }
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
// cerr << "length = " << iter.first << ", samples = " << limit << "/" << group.size() << endl;
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

inline vector<Pattern> generateAll(string LABEL_METHOD, string LABEL_FILE, string ALL_FILE, string QUALITY_FILE)
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

    if (LABEL_METHOD.find("D") != -1) { // distant training
        bool needPos = LABEL_METHOD.find("DP") != -1;
        bool needNeg = LABEL_METHOD.find("DN") != -1;

        unordered_set<ULL> include = loadPatterns(QUALITY_FILE, MAX_POSITIVE);
        unordered_set<ULL> exclude = loadPatterns(ALL_FILE, MAX_POSITIVE);

        if (MAX_POSITIVE != -1) {
            exclude.clear();
        }

        for (ULL value : include) { // make sure exclude is a super set of include
            exclude.insert(value);
        }
        for (int i = 0; i < ret.size(); ++ i) { // make sure every human label is excluded
            exclude.insert(ret[i].hashValue);
        }

        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            if (patterns[i].size() < 1) {
                continue;
            }
            if (include.count(patterns[i].hashValue)) {
                if (needPos) {
                    ret.push_back(patterns[i]);
                    ret.back().label = 1;
                }
            } else if (!exclude.count(patterns[i].hashValue)) {
                if (needNeg) {
                    ret.push_back(patterns[i]);
                    ret.back().label = 0;
                }
            }
        }
    }

    int cntPositives = 0, cntNegatives = 0;
    for (PATTERN_ID_TYPE i = 0; i < ret.size(); ++ i) {
        if (ret[i].label == 1) {
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

inline vector<Pattern> generate_tag(vector<vector<double>> &features, string QUALITY_FILE, bool isEntity)
{
    unordered_set<ULL> exclude = loadPatterns_tag(QUALITY_FILE);
    //unordered_set<ULL> include = loadPatterns_tag(QUALITY_FILE);

    vector<int> positives, negatives;
    for (int i = 0; i < patterns_tag.size(); ++ i) {
        if (patterns_tag[i].size() <= 1) {
            continue;
        }
        //assert(features[i].size() != 0);
        if (exclude.count(patterns_tag[i].hashValue)) {
            positives.push_back(i);
        } else if (!exclude.count(patterns_tag[i].hashValue)) {
            negatives.push_back(i);
        }
    }
    
    
    fprintf(stderr, "matched positives = %d\n", positives.size());
    fprintf(stderr, "matched negatives = %d\n", negatives.size());

    random_shuffle(positives.begin(), positives.end());
    if (MAX_POSITIVE > 0 && MAX_POSITIVE < positives.size()) {
        positives.resize(MAX_POSITIVE);
    }

    int REAL_POSITIVE=positives.size();
    //for (int i = 0; i < positives.size(); ++i)
    //{
        /* code */
    //    patterns_tag[positives[i]].showtag();
    //}


    random_shuffle(negatives.begin(), negatives.end());
    if (REAL_POSITIVE > 0 && REAL_POSITIVE*NEGATIVE_RATIO < negatives.size()) {
        negatives.resize(REAL_POSITIVE*NEGATIVE_RATIO);
    }

    fprintf(stderr, "selected positives = %d\n", positives.size());
    fprintf(stderr, "selected negatives = %d\n", negatives.size());
    
    vector<Pattern> ret;
    int posLabel;
    if (isEntity)
        posLabel=1;
    else
        posLabel=2;
    for (int id : positives) {
        ret.push_back(patterns_tag[id]);
        ret.back().label = posLabel;
    }
    for (int id : negatives) {
        ret.push_back(patterns_tag[id]);
        ret.back().label = 0;
    }

    return ret;
}

inline vector<int> generate_samples(string QUALITY_FILE){
    unordered_set<ULL> exclude = loadPatterns_tag(QUALITY_FILE);
    //unordered_set<ULL> include = loadPatterns_tag(QUALITY_FILE);

    vector<int> positives, negatives;
    for (int i = 0; i < patterns_tag.size(); ++ i) {
        if (patterns_tag[i].size() <= 1) {
            continue;
        }
        if (exclude.count(patterns_tag[i].hashValue)) {
            positives.push_back(i);
        } else if (!exclude.count(patterns_tag[i].hashValue)) {
            negatives.push_back(i);
        }
    }
    

    //random_shuffle(positives.begin(), positives.end());
    //if (MAX_POSITIVE > 0 && MAX_POSITIVE < positives.size()) {
    //    positives.resize(MAX_POSITIVE);
    //}

    random_shuffle(negatives.begin(), negatives.end());
    if (10000 < negatives.size()) {
        negatives.resize(10000);
    }

    fprintf(stderr, "selected positives = %d\n", positives.size());
    fprintf(stderr, "selected negatives = %d\n", negatives.size());
    
    vector<int> ret;
    for (int id : positives) {
        ret.push_back(id);
    }
    for (int id : negatives) {
        ret.push_back(id);
    }

    return ret;
}

/*
inline vector<Pattern> generateUnigram(string ALL_FILE, string QUALITY_FILE)
{
    unordered_set<ULL> exclude = loadPatterns(ALL_FILE);
    unordered_set<ULL> include = loadPatterns(QUALITY_FILE);

    vector<pair<ULL, PATTERN_ID_TYPE>> positiveOrder, negativeOrder;
    for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
        if (patterns[i].size() != 1 || unigrams[patterns[i].tokens[0]] < MIN_SUP) {
            continue;
        }
        if (include.count(patterns[i].hashValue)) {
            positiveOrder.push_back(make_pair(patterns[i].hashValue, i));
        } else if (!exclude.count(patterns[i].hashValue)) {
            negativeOrder.push_back(make_pair(patterns[i].hashValue, i));
        }
    }
    sort(positiveOrder.begin(), positiveOrder.end());
    sort(negativeOrder.begin(), negativeOrder.end());
    // make sure the patterns are same each time
    vector<PATTERN_ID_TYPE> positives, negatives;
    for (const auto& iter : positiveOrder) {
        positives.push_back(iter.second);
    }
    for (const auto& iter : negativeOrder) {
        negatives.push_back(iter.second);
    }

    fprintf(stderr, "matched positives = %d\n", positives.size());
    fprintf(stderr, "matched negatives = %d\n", negatives.size());

    vector<Pattern> ret;
    int cntPositives = 0, cntNegatives = 0;
    srand(19910724);

    random_shuffle(positives.begin(), positives.end());
    if (MAX_POSITIVE > 0 && MAX_POSITIVE < positives.size()) {
        positives.resize(MAX_POSITIVE);
    }

    // positives
    for (PATTERN_ID_TYPE id : positives) {
        ret.push_back(patterns[id]);
        ret.back().label = 1;
        ++ cntPositives;
    }

    // negatives part 1
    random_shuffle(negatives.begin(), negatives.end());
    if (positives.size() * NEGATIVE_RATIO < negatives.size()) {
        negatives.resize(positives.size() * NEGATIVE_RATIO);
    }

    for (PATTERN_ID_TYPE id : negatives) {
        ret.push_back(patterns[id]);
        ret.back().label = 0;
        ++ cntNegatives;
    }

    fprintf(stderr, "selected positives = %d\n", cntPositives);
    fprintf(stderr, "selected negatives = %d\n", cntNegatives);

    return ret;
}
*/

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
    for (Pattern& pattern : patterns_tag) {
        if (pattern.currentFreq == 0 && pattern.label == 1) {
            pattern.label = FrequentPatternMining::UNKNOWN_LABEL;
            for (int i = 0; i < pattern.size(); ++i)
            {
                cerr<<posid2Tag[pattern.tokens[i]]<<" ";
            }
            cerr<<endl;
            ++ cnt;
        }
    }
}

}

#endif
