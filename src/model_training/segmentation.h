#ifndef __SEGMENTATION_H__
#define __SEGMENTATION_H__

#include <cassert>

#include "../utils/utils.h"
#include "../frequent_pattern_mining/frequent_pattern_mining.h"
using FrequentPatternMining::Pattern;
// === global variables ===
using FrequentPatternMining::patterns;
using FrequentPatternMining::pattern2id;
using FrequentPatternMining::id2ends;
using FrequentPatternMining::unigrams;

using FrequentPatternMining::patterns_tag;
using FrequentPatternMining::id2ends_tag;
using FrequentPatternMining::unigrams_tag;
using Documents::posid2Tag;

mutex POSTagMutex[SUFFIX_MASK + 1];

struct TrieNode {
    unordered_map<TOTAL_TOKENS_TYPE, size_t> children;

    PATTERN_ID_TYPE id;
    string indicator;

    TrieNode() {
        id = -1;
        indicator = "None";
        children.clear();
    }
};

vector<TrieNode> trie;
vector<TrieNode> trie_pos;

// ===

void constructTrie() {
    trie.clear();
    trie.push_back(TrieNode());
    for (int i = 0; i < patterns.size(); ++ i) {
        const vector<TOTAL_TOKENS_TYPE>& tokens = patterns[i].tokens;
        if (tokens.size() == 0 || tokens.size() > 1 && patterns[i].currentFreq == 0) {
            continue;
        }
        size_t u = 0;
        for (const TOTAL_TOKENS_TYPE& token : tokens) {
            if (!trie[u].children.count(token)) {
                trie[u].children[token] = trie.size();
                trie.push_back(TrieNode());
            }
            u = trie[u].children[token];
        }
        trie[u].id = i;
        trie[u].indicator = patterns[i].indicator;
        //cerr<<patterns[i].postagquality<<endl;
    }
    cerr << "# of trie nodes = " << trie.size() << endl;
}

void constructTrie_pos() {
    trie_pos.clear();
    trie_pos.push_back(TrieNode());
    for (int i = 0; i < patterns_tag.size(); ++ i) {
        const vector<TOTAL_TOKENS_TYPE>& tokens = patterns_tag[i].tokens;
        if (tokens.size() == 0 || tokens.size() > 1 && patterns_tag[i].currentFreq == 0) {
            continue;
        }
        size_t u = 0;
        for (const TOTAL_TOKENS_TYPE& token : tokens) {
            if (!trie_pos[u].children.count(token)) {
                trie_pos[u].children[token] = trie_pos.size();
                trie_pos.push_back(TrieNode());
            }
            u = trie_pos[u].children[token];
        }
        trie_pos[u].id = i;
    }
    cerr << "# of pos tag trie nodes = " << trie_pos.size() << endl;
}

class Segmentation
{
private:
    static const double INF;
    static vector<vector<TOTAL_TOKENS_TYPE>> total;
    static vector<TOTAL_TOKENS_TYPE> tree_total;

public:
    static bool ENABLE_POS_TAGGING;
    static double penalty;
    static vector<vector<double>> connect, disconnect;
    
    // dependency tree maps
    static map<string, int> tree_map;
    static vector<double> deps_prob;

    static string treeToString(const vector<vector<int>>& children, int u) {
        vector<string> subtrees;
        for (int v : children[u]) {
            subtrees.push_back(treeToString(children, v));
        }
        sort(subtrees.begin(), subtrees.end());
        string ret = "(x";
        for (const string& subtree : subtrees) {
            ret += subtree;

        }
        ret += ")";
        return ret;
    }

    static int GetSubtreeID(const vector<int> &deps, int start, int end) {
        vector<vector<int>> children(deps.size() + 1);
        vector<bool> isRoot(deps.size(), true);
        for (int i = start; i < end; ++ i) {
            int a = i + 1, b = deps[i];
            if (b > start && b <= end) {
                children[b].push_back(a);
                isRoot[a] = false;
                // isRoot[b] = false;
            }
        }

        for (int i = 1 + start; i < 1 + end; ++ i) {
            if (isRoot[i]) {
                children[0].push_back(i);
            }
        }

        string min_representation = treeToString(children, 0);
        assert(tree_map.count(min_representation));
        return tree_map[min_representation];
    }

    static int InsertOrGetSubtreeID(const vector<int> &deps, int start, int end) {
        vector<vector<int>> children(deps.size() + 1);
        vector<bool> isRoot(deps.size(), true);
        for (int i = start; i < end; ++ i) {
            int a = i + 1, b = deps[i];
            if (b > start && b <= end) {
                children[b].push_back(a);
                isRoot[a] = false;
                // isRoot[b] = false;
            }
        }

        for (int i = 1 + start; i < 1 + end; ++ i) {
            if (isRoot[i]) {
                children[0].push_back(i);
            }
        }

        string min_representation = treeToString(children, 0);
        if (tree_map.count(min_representation)) {
            return tree_map[min_representation];
        }
        int new_id = tree_map.size();
        assert(new_id == deps_prob.size());
        if (children[0].size() == 1)
            deps_prob.push_back(1);
        else
            deps_prob.push_back(0);
        return tree_map[min_representation] = new_id;
    }

    static void initializePosTags(int n) {
        // uniformly initialize
        connect.resize(n);
        for (int i = 0; i < n; ++ i) {
            connect[i].resize(n);
            for (int j = 0; j < n; ++ j) {
                connect[i][j] = 1.0 / n;
            }
        }
        // getDisconnect();
        total = vector<vector<TOTAL_TOKENS_TYPE>>(connect.size(), vector<TOTAL_TOKENS_TYPE>(connect.size(), 0));
        for (TOTAL_TOKENS_TYPE i = 1; i < Documents::totalWordTokens - 1; ++ i) {
            if (!Documents::isEndOfSentence(i)) {
                ++ total[Documents::posTags[i - 1]][Documents::posTags[i+1]];
            }
        }
    }

    static void initializeDeps(vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> &sentences, int LENGTH_THRESHOLD) {
        // TODO(branzhu): intialize by whole corpus
        deps_prob.clear();
        tree_map.clear();
        tree_total.clear();

        for (INDEX_TYPE senID = 0; senID < sentences.size(); ++ senID) {
            vector<TOKEN_ID_TYPE> deps;
            for (TOTAL_TOKENS_TYPE i = sentences[senID].first; i <= sentences[senID].second; ++ i) {
                deps.push_back(Documents::depPaths[i]);
            }
            for (int len = 2; len <= LENGTH_THRESHOLD; ++len) {
                for (int st = 0; st + len <= deps.size(); ++st) {
                    int index = InsertOrGetSubtreeID(deps, st, st+len);
                    assert(index <= tree_total.size());
                    if (index == tree_total.size()) {
                        tree_total.push_back(1);
                    }
                    else {
                        ++ tree_total[index];
                    }
                }
            }
        }
    }

    static double GetPuncCost(const vector<TOKEN_ID_TYPE> &tokens, const vector<TOKEN_ID_TYPE> &tags, int start, int end) {
        if (Documents::punctuations.count(tokens[start])) {
            if  (Documents::punctuations[tokens[start]] != "(" && Documents::punctuations[tokens[start]] != "\'\'")
                return -INF;
            if (!Documents::punctuations.count(tokens[end])) return -INF;
        }

        if (Documents::punctuations.count(tokens[end])) {
            if (Documents::punctuations[tokens[end]] == ")" && Documents::punctuations[tokens[start]] != "(")
                return -INF;
            if (Documents::punctuations[tokens[end]] == "\'\'" && Documents::punctuations[tokens[start]] != "\'\'")
                return -INF;
            if (Documents::punctuations[tokens[end]] != ")" && Documents::punctuations[tokens[end]] != "\'\'")
                return -INF;
        }
        // cerr << "Get One" << endl;
        double PCost = 0;
        for (int i = start + 1; i < end ; ++i) {
          if (Documents::isPunc(tokens[i])) {
            PCost += log(connect[tags[i-1]][tags[i+1]] + EPS);
          }   
        }
        return PCost;
    }

    static void getDisconnect() {
        disconnect = connect;
        for (int i = 0; i < connect.size(); ++ i) {
            for (int j = 0; j < connect[i].size(); ++ j) {
                disconnect[i][j] = 1 - connect[i][j];
            }
        }
    }

    static void normalizePosTags() {
        for (int i = 0; i < connect.size(); ++ i) {
            double sum = 0;
            for (int j = 0; j < connect[i].size(); ++ j) {
                sum += connect[i][j];
            }
            if (sum > EPS) {
                for (int j = 0; j < connect[i].size(); ++ j) {
                    connect[i][j] /= sum;
                }
            } else {
                for (int j = 0; j < connect[i].size(); ++ j) {
                    connect[i][j] = 1.0 / (double)connect[i].size();
                }
            }
        }
        getDisconnect();
    }

    static void logPosTags() {
        for (int i = 0; i < connect.size(); ++ i) {
            for (int j = 0; j < connect[i].size(); ++ j) {
                connect[i][j] = log(connect[i][j] + EPS);
                disconnect[i][j] = log(disconnect[i][j] + EPS);
            }
        }
    }

    static void logDeps() {
        for (int i = 0; i < deps_prob.size(); ++i) {
            deps_prob[i] = log(deps_prob[i] + EPS);
        }
    }

private:
    // generated
    int maxLen;
    double *prob;

    void normalize() {
        vector<double> sum(maxLen + 1, 0);
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            sum[patterns[i].size()] += prob[i];
        }
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            prob[i] /= sum[patterns[i].size()];
        }
    }

    void initialize() {
        // compute maximum tokens
        maxLen = 0;
        for (int i = 0; i < patterns.size(); ++ i) {
            maxLen = max(maxLen, patterns[i].size());
        }

        prob = new double[patterns.size()];
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            prob[i] = 0;
        }
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            prob[i] = patterns[i].currentFreq;
        }
        normalize();
    }

public:

    double getProb(int id) const {
        return exp(prob[id]);
    }

    int getSize() const {
        return patterns.size();
    }

    double* getProb() const{
        return prob;
    }

    ~Segmentation() {
        delete [] prob;
    }

    //TODO(branzhu): Change this into Enable Constraints Mode.
    Segmentation(bool ENABLE_POS_TAGGING) {
        assert(ENABLE_POS_TAGGING == true);
        Segmentation::ENABLE_POS_TAGGING = ENABLE_POS_TAGGING;
        initialize();
        for (int i = 0; i < patterns.size(); ++ i) {
            prob[i] = log(prob[i] + EPS) + log(patterns[i].quality + EPS);
        }
    }
    //segmentation penalty
    Segmentation(double penalty) {
        Segmentation::penalty = penalty;
        initialize();
        // P(length)
        vector<double> pLen(maxLen + 1, 1);
        double total = 1;
        for (int i = 1; i <= maxLen; ++ i) {
            pLen[i] = pLen[i - 1] / penalty;
            total += pLen[i];
        }
        for (int i = 0; i <= maxLen; ++ i) {
            pLen[i] /= total;
        }

        for (int i = 0; i < patterns.size(); ++ i) {
            prob[i] = log(prob[i] + EPS) + log(pLen[patterns[i].size() - 1]) + log(patterns[i].quality + EPS);
        }
    }

    inline double viterbi(const vector<TOKEN_ID_TYPE> &tokens, vector<double> &f, vector<int> &pre) {
        f.clear();
        f.resize(tokens.size() + 1, -INF);
        pre.clear();
        pre.resize(tokens.size() + 1, -1);
        f[0] = 0;
        pre[0] = 0;
        for (size_t i = 0 ; i < tokens.size(); ++ i) {
            if (f[i] < -1e80) {
                continue;
            }
            bool impossible = true;
            for (size_t j = i, u = 0; j < tokens.size(); ++ j) {
                if (!trie[u].children.count(tokens[j])) {
                    break;
                }
                u = trie[u].children[tokens[j]];
                if (trie[u].id != -1) {
                    impossible = false;
                    PATTERN_ID_TYPE id = trie[u].id;
                    double p = prob[id];
                    if (f[i] + p > f[j + 1]) {
                        f[j + 1] = f[i] + p;
                        pre[j + 1] = i;
                    }
                }
            }
            if (impossible) {
                if (f[i] > f[i + 1]) {
                    f[i + 1] = f[i];
                    pre[i + 1] = i;
                }
            }
        }
        return f[tokens.size()];
    }

    inline double viterbi_proba(const vector<TOKEN_ID_TYPE> &tokens, vector<double> &f, vector<int> &pre) {
        f.clear();
        f.resize(tokens.size() + 1, 0);
        pre.clear();
        pre.resize(tokens.size() + 1, -1);
        f[0] = 1;
        pre[0] = 0;
        for (size_t i = 0 ; i < tokens.size(); ++ i) {
            Pattern pattern;
            for (size_t j = i, u = 0; j < tokens.size(); ++ j) {
                if (!trie[u].children.count(tokens[j])) {
                    if (j == i) {
                        if (f[i] > f[j + 1]) {
                            f[j + 1] = f[i];
                            pre[j + 1] = i;
                        }
                    }
                    break;
                }
                u = trie[u].children[tokens[j]];
                if (trie[u].id != -1 && j - i + 1 != tokens.size()) {
                    PATTERN_ID_TYPE id = trie[u].id;
                    double p = exp(prob[id]);
                    if (f[i] * p > f[j + 1]) {
                        f[j + 1] = f[i] * p;
                        pre[j + 1] = i;
                    }
                }
            }
        }
        return f[tokens.size()];
    }

    inline bool istree(const vector<TOKEN_ID_TYPE> &deps, int start, int end) {
        int out_edge = 0;
        if (start == end) return true;
        for (int i = start; i <= end; ++i) {
            // cerr << out_edge << " i=" << i;
            if (deps[i] <= start || deps[i] > i+1) {
                if (out_edge == i+1 || out_edge < 1){
                    out_edge = deps[i];
                }
                else {
                    return false;
                }
            }
        }
        return true;
    }

    inline double viterbi(const vector<TOKEN_ID_TYPE> &tokens, const vector<TOKEN_ID_TYPE> &deps, const vector<TOKEN_ID_TYPE>& tags, vector<double> &f, vector<int> &pre) {
        f.clear();
        f.resize(tokens.size() + 1, -INF);
        pre.clear();
        pre.resize(tokens.size() + 1, -1);
        f[0] = 0;
        pre[0] = 0;
        assert(tokens.size() == deps.size());
        for (size_t i = 0 ; i < tokens.size(); ++ i) {
            if (f[i] < -1e80) {
                continue;
            }
            Pattern pattern;
            double cost = 0;
            bool impossible = true;
            for (size_t j = i, u = 0; j < tokens.size(); ++ j) {
                if (!trie[u].children.count(tokens[j])) {
                    break;
                }
                u = trie[u].children[tokens[j]];
                if (trie[u].id != -1) {
                    impossible = false;
                    PATTERN_ID_TYPE id = trie[u].id;
                    double p = cost + prob[id];
                    // double depCost = istree(deps, i, j) ? 0 : -INF;
                    double multiConstraints = 0.0;
                    
                    // TODO(branzhu): incorporate 
                    if (j > i) {
                        int index = GetSubtreeID(deps, i, j+1);
                        multiConstraints += deps_prob[index];
                    }
                    // TODO(branzhu): add punc cost 
                    
                    if (j > i) {
                        multiConstraints += GetPuncCost(tokens, tags, i, j);
                        // at the corner 1e-100 else 1e-20
                    }


                    // cerr << i<<tokens[i] << " " << j<<tokens[j] << " " << deps[i] << " "<< depCost << endl;
                    // double tagCost = (j + 1 < tokens.size() && tags[j] >= 0 && tags[j + 1] >= 0) ? disconnect[tags[j]][tags[j + 1]] : 0;
                    if (f[i] + p + multiConstraints > f[j + 1]) {
                        f[j + 1] = f[i] + p + multiConstraints;
                        pre[j + 1] = i;
                    }
                }
            }
            if (impossible) {
                if (f[i] > f[i + 1]) {
                    f[i + 1] = f[i];
                    pre[i + 1] = i;
                }
            }
        }
        return f[tokens.size()];
    }

    inline double viterbi_proba_randomPOS(const vector<TOKEN_ID_TYPE> &tokens, vector<double> &f, vector<int> &pre) {
        Pattern pattern;
        for (int i = 0; i < tokens.size(); ++ i) {
            pattern.append(tokens[i]);
        }
        assert(pattern2id.count(pattern.hashValue));
        PATTERN_ID_TYPE id = pattern2id[pattern.hashValue];

        double sum = 0;
        map<vector<POS_ID_TYPE>, double> memo;
        for (TOTAL_TOKENS_TYPE ed : id2ends[id]) {
            TOTAL_TOKENS_TYPE st = ed - pattern.size() + 1;
            vector<POS_ID_TYPE> tags;
            for (int i = st; i <= ed; ++ i) {
                tags.push_back(Documents::posTags[i]);
            }
            if (memo.count(tags)) {
                sum += memo[tags];
                continue;
            }
            f.clear();
            f.resize(tokens.size() + 1, 0);
            pre.clear();
            pre.resize(tokens.size() + 1, -1);
            f[0] = 1;
            pre[0] = 0;
            for (size_t i = 0 ; i < tokens.size(); ++ i) {
                double cost = 1;
                for (size_t j = i, u = 0; j < tokens.size() && j - i + 1 < tokens.size(); ++ j) {
                    if (!trie[u].children.count(tokens[j])) {
                        assert(j != i);
                        break;
                    }
                    u = trie[u].children[tokens[j]];
                    if (trie[u].id != -1) {
                        PATTERN_ID_TYPE id = trie[u].id;
                        double p = cost * exp(prob[id]);
                        double tagP = (j + 1 < tokens.size() && tags[j] >= 0 && tags[j + 1] >= 0) ? disconnect[tags[j]][tags[j + 1]] : 1;
                        if (f[i] * p * tagP > f[j + 1]) {
                            f[j + 1] = f[i] * p * tagP;
                            pre[j + 1] = i;
                        }
                    }
                    if (j + 1 < tokens.size() && tags[j] >= 0 && tags[j + 1] >= 0) {
                        cost *= connect[tags[j]][tags[j + 1]];
                    }
                }
            }
            memo[tags] = f[tokens.size()];
            sum += f[tokens.size()];
        }
        return sum / id2ends[id].size();
    }

    inline double rectifyFrequency(vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> &sentences) {
        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            patterns[i].currentFreq = 0;
            id2ends[i].clear();
        }

        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (PATTERN_ID_TYPE i = 0; i < patterns_tag.size(); ++ i) {
            patterns_tag[i].currentFreq = 0;
            id2ends_tag[i].clear();
        }

        double energy = 0;
        # pragma omp parallel for reduction(+:energy) schedule(dynamic, SENTENCE_CHUNK_SIZE)
        for (INDEX_TYPE senID = 0; senID < sentences.size(); ++ senID) {
            vector<TOKEN_ID_TYPE> tokens;
            vector<TOKEN_ID_TYPE> postags;
            for (TOTAL_TOKENS_TYPE i = sentences[senID].first; i <= sentences[senID].second; ++ i) {
                tokens.push_back(Documents::wordTokens[i]);
                postags.push_back(Documents::posTags[i]);
            }
            vector<double> f;
            vector<int> pre;

            double bestExplain = viterbi(tokens, f, pre);

            int i = (int)tokens.size();
            assert(f[i] > -1e80);

            energy += f[i];

    		while (i > 0) {
    			int j = pre[i];
                size_t u = 0;
                for (int k = j; k < i; ++ k) {
                    assert(trie[u].children.count(tokens[k]));
                    u = trie[u].children[tokens[k]];
                }
                if (trie[u].id != -1) {
                    PATTERN_ID_TYPE id = trie[u].id;
                    separateMutex[id & SUFFIX_MASK].lock();
                    ++ patterns[id].currentFreq;
                    if (i - j > 1 || i - j == 1 && unigrams[patterns[id].tokens[0]] >= MIN_SUP) {
                        id2ends[id].push_back(sentences[senID].first + i - 1);
                    }
                    separateMutex[id & SUFFIX_MASK].unlock();
                }
                u = 0;
                bool local_mis = false;
                for (int k = j; k < i; ++ k) {
                    if (trie_pos[u].children.count(postags[k]) > 0) {
                        u = trie_pos[u].children[postags[k]];
                    }
                    else {
                        local_mis = true;
                        // mistakes += 1;
                        // i = j;
                        break;
                    }
                }
                if (!local_mis && trie_pos[u].id != -1) {
                    PATTERN_ID_TYPE id = trie_pos[u].id;
                    separateMutex[id & SUFFIX_MASK].lock();
                    ++ patterns_tag[id].currentFreq;
                    if (i - j > 1 || i - j == 1 && unigrams_tag[patterns_tag[id].tokens[0]] >= MIN_SUP) {
                        id2ends_tag[id].push_back(sentences[senID].first + i - 1);
                    }
                    separateMutex[id & SUFFIX_MASK].unlock();
                }
    			i = j;
    		}
        }

        // cerr << "Trie search mistakes:" << mistakes << ":" << total << endl;
        cerr << "Energy = " << energy << endl;
        return energy;
    }

    inline double rectifyFrequencyDeps(vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> &sentences) {
        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (PATTERN_ID_TYPE i = 0; i < patterns.size(); ++ i) {
            patterns[i].currentFreq = 0;
            id2ends[i].clear();
        }

        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (PATTERN_ID_TYPE i = 0; i < patterns_tag.size(); ++ i) {
            patterns_tag[i].currentFreq = 0;
            id2ends_tag[i].clear();
        }

        logDeps();

        double energy = 0;
        # pragma omp parallel for reduction(+:energy) schedule(dynamic, SENTENCE_CHUNK_SIZE)
        for (INDEX_TYPE senID = 0; senID < sentences.size(); ++ senID) {
            vector<TOKEN_ID_TYPE> tokens;
            vector<TOKEN_ID_TYPE> postags;
            vector<TOKEN_ID_TYPE> deps;
            for (TOTAL_TOKENS_TYPE i = sentences[senID].first; i <= sentences[senID].second; ++ i) {
                tokens.push_back(Documents::wordTokens[i]);
                postags.push_back(Documents::posTags[i]);
                deps.push_back(Documents::depPaths[i]);
            }
            vector<double> f;
            vector<int> pre;

            double bestExplain = viterbi(tokens, deps, postags, f, pre);

            int i = (int)tokens.size();
            assert(f[i] > -1e80);

            energy += f[i];

            while (i > 0) {
                int j = pre[i];
                size_t u = 0;
                for (int k = j; k < i; ++ k) {
                    assert(trie[u].children.count(tokens[k]));
                    u = trie[u].children[tokens[k]];
                }
                if (trie[u].id != -1) {
                    PATTERN_ID_TYPE id = trie[u].id;
                    separateMutex[id & SUFFIX_MASK].lock();
                    ++ patterns[id].currentFreq;
                    if (i - j > 1 || i - j == 1 && unigrams[patterns[id].tokens[0]] >= MIN_SUP) {
                        id2ends[id].push_back(sentences[senID].first + i - 1);
                    }
                    separateMutex[id & SUFFIX_MASK].unlock();
                }
                u = 0;
                bool local_mis = false;
                for (int k = j; k < i; ++ k) {
                    if (trie_pos[u].children.count(postags[k]) > 0) {
                        u = trie_pos[u].children[postags[k]];
                    }
                    else {
                        local_mis = true;
                        // mistakes += 1;
                        // i = j;
                        break;
                    }
                }
                if (!local_mis && trie_pos[u].id != -1) {
                    PATTERN_ID_TYPE id = trie_pos[u].id;
                    separateMutex[id & SUFFIX_MASK].lock();
                    ++ patterns_tag[id].currentFreq;
                    if (i - j > 1 || i - j == 1 && unigrams_tag[patterns_tag[id].tokens[0]] >= MIN_SUP) {
                        id2ends_tag[id].push_back(sentences[senID].first + i - 1);
                    }
                    separateMutex[id & SUFFIX_MASK].unlock();
                }
                i = j;
            }
        }

        // cerr << "Trie search mistakes:" << mistakes << ":" << total << endl;
        cerr << "Energy = " << energy << endl;
        return energy;
    }

    inline double adjustConstraints(vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> &sentences, int MIN_SUP) {
        assert(tree_map.size() == deps_prob.size());
        vector<double> tree_cnt(tree_map.size(), 0);
        
        logDeps();

        double energy = 0;
        # pragma omp parallel for reduction(+:energy) schedule(dynamic, SENTENCE_CHUNK_SIZE)
        for (INDEX_TYPE senID = 0; senID < sentences.size(); ++ senID) {
            vector<TOKEN_ID_TYPE> tokens;
            vector<TOKEN_ID_TYPE> tags;
            vector<TOKEN_ID_TYPE> deps;
            for (TOTAL_TOKENS_TYPE i = sentences[senID].first; i <= sentences[senID].second; ++ i) {
                tokens.push_back(Documents::wordTokens[i]);
                tags.push_back(Documents::posTags[i]);
                deps.push_back(Documents::depPaths[i]);
            }
            vector<double> f;
            vector<int> pre;

            double bestExplain = viterbi(tokens, deps, tags, f, pre);

            int i = (int)tokens.size();
            assert(f[i] > -1e80);
            energy += f[i];
    		while (i > 0) {
    			int j = pre[i];
                size_t u = 0;
                for (int k = j; k < i; ++ k) {
                    if (!trie[u].children.count(tokens[k])) {
                        for (const auto& t : tokens) {
                            cerr << t << " ";
                        }
                    }
                    assert(trie[u].children.count(tokens[k]));
                    u = trie[u].children[tokens[k]];
                }
                if (trie[u].id != -1) {
                    if (i - j > 1) {
                        // TODO(branzhu): support multi-threads version
                        int index = GetSubtreeID(deps, j, i);
                        // int id = index.tree_map.size() 
                        // if (index > tree_map.size()) cerr << "careful" << endl;
                        POSTagMutex[index & SUFFIX_MASK].lock();
                        ++ tree_cnt[index];
                        POSTagMutex[index & SUFFIX_MASK].unlock();
                    }
                }
    			i = j;
    		}
        }

        for (int i = 0; i < tree_map.size(); ++i) {
            deps_prob[i] = tree_cnt[i] / tree_total[i];
        }

        cerr << "Energy = " << energy << endl;
        return energy;
    }
};

const double Segmentation::INF = 1e100;
vector<vector<double>> Segmentation::connect;
vector<vector<double>> Segmentation::disconnect;
map<string, int> Segmentation::tree_map;
vector<double> Segmentation::deps_prob;
vector<TOTAL_TOKENS_TYPE> Segmentation::tree_total;
vector<vector<TOTAL_TOKENS_TYPE>> Segmentation::total;
bool Segmentation::ENABLE_POS_TAGGING;
double Segmentation::penalty;
#endif
