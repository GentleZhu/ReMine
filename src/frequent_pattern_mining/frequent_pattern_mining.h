#ifndef __FREQUENT_PATTERN_MINING_H__
#define __FREQUENT_PATTERN_MINING_H__

#include "../utils/utils.h"
#include "../data/documents.h"
using Documents::posid2Tag;
namespace FrequentPatternMining
{
    const ULL MAGIC = 0xabcdef;
    const int UNKNOWN_LABEL = -1000000000;

    struct Pattern {
        vector<TOKEN_ID_TYPE> tokens;
        vector<TOKEN_ID_TYPE> postags;
        int label;
        double probability, quality;
        ULL hashValue;
        int currentFreq;
        string indicator;

        void dump(FILE* out) {
            Binary::write(out, currentFreq);
            Binary::write(out, quality);
            //cerr<<postagquality<<endl;
            Binary::write(out, indicator);
            Binary::write(out, tokens.size());
            for (auto& token : tokens) {
                Binary::write(out, token);
            }
            //TODO(Bran): Output 
        }

        void load(FILE* in) {
            Binary::read(in, currentFreq);
            Binary::read(in, quality);
            //cerr<<postagquality<<endl;
            Binary::read(in, indicator);
            size_t tokenSize;
            Binary::read(in, tokenSize);
            tokens.clear();
            for (size_t i = 0; i < tokenSize; ++ i) {
                TOTAL_TOKENS_TYPE token;
                Binary::read(in, token);
                append(token);
            }
        }

        Pattern(const TOKEN_ID_TYPE &token) {
            tokens.clear();
            hashValue = 0;
            currentFreq = 0;
            label = UNKNOWN_LABEL;
            append(token);
            quality = 1;
            indicator = "None";
        }

        Pattern(const TOKEN_ID_TYPE &token,const TOKEN_ID_TYPE &postag) {
            tokens.clear();
            postags.clear();
            hashValue = 0;
            currentFreq = 0;
            label = UNKNOWN_LABEL;
            appendwithpos(token,postag);
            quality = 1;
            indicator = "None";
            //postags.push_back(postag);
        }

        Pattern(const Pattern &other) {
            this->tokens = other.tokens;
            //
            if (other.postags.size())
                this->postags = other.postags;
            //
            this->hashValue = other.hashValue;
            this->probability = other.probability;
            this->label = other.label;
            this->quality = other.quality;
            this->currentFreq = other.currentFreq;
            this->indicator = other.indicator;
        }

        Pattern() {
            tokens.clear();
            postags.clear();
            hashValue = 0;
            currentFreq = 0;
            label = UNKNOWN_LABEL;
            quality = 1;
            indicator = "None";
        }

        inline void shrink_to_fit() {
            tokens.shrink_to_fit();
            postags.shrink_to_fit();
        }

        inline Pattern substr(int l, int r) const {
            Pattern ret;
            for (int i = l; i < r; ++ i)  {
                ret.append(tokens[i]);
            }
            return ret;
        }

        inline int size() const {
            return tokens.size();
        }

        inline ULL getHash() const {
            ULL pos_hashvalue = 0;
            for (int i = 0; i < postags.size(); ++i)
                pos_hashvalue = pos_hashvalue * MAGIC + postags[i] + 1;
            return pos_hashvalue;
        }

        inline void append(const TOKEN_ID_TYPE &token) {
            tokens.push_back(token);
            hashValue = hashValue * MAGIC + token + 1;
        }

        inline void appendwithpos(const TOKEN_ID_TYPE &token, const TOKEN_ID_TYPE &postag){
            tokens.push_back(token);
            postags.push_back(postag);
            hashValue = hashValue * MAGIC + token + 1;

        }

        inline bool operator == (const Pattern &other) const {
            return hashValue == other.hashValue && tokens == other.tokens;
        }

        inline void show() const {
            for (int i = 0; i < tokens.size(); ++ i) {
                cerr << tokens[i] << " ";
            }
            cerr << endl;
        }
        inline void showtag() const {
            for (int i = 0; i < tokens.size(); ++ i) {
                cerr << posid2Tag[tokens[i]] << " ";
            }
            cerr << endl;
        }
    };

// === global variables ===
    TOTAL_TOKENS_TYPE *unigrams; // 0 .. Documents::maxTokenID
    vector<Pattern> patterns;
    vector<vector<TOTAL_TOKENS_TYPE>> id2ends;
    unordered_map<ULL, PATTERN_ID_TYPE> pattern2id;

    TOTAL_TOKENS_TYPE *unigrams_tag; // 0 .. Documents::maxTokenID
    vector<Pattern> patterns_tag;
    vector<vector<TOTAL_TOKENS_TYPE>> id2ends_tag;
    unordered_map<ULL, PATTERN_ID_TYPE> pattern2id_tag;

// ===

    inline void addPatternWithoutLocks(const Pattern& pattern, const TOTAL_TOKENS_TYPE& ed, bool addPosition = true) {
        assert(pattern2id.count(pattern.hashValue));
        PATTERN_ID_TYPE id = pattern2id[pattern.hashValue];
        assert(id < id2ends.size());
        if (patterns[id].currentFreq == 0) {
            patterns[id] = pattern;
        }
        if (addPosition) {
            assert(patterns[id].currentFreq < id2ends[id].size());
            id2ends[id][patterns[id].currentFreq] = ed;
        }
        ++ patterns[id].currentFreq;
    }

    inline void addPatternWithoutLocksPos(const Pattern& pattern, const TOTAL_TOKENS_TYPE& ed, bool addPosition = true) {
        assert(pattern2id_tag.count(pattern.hashValue));
        PATTERN_ID_TYPE id = pattern2id_tag[pattern.hashValue];
        assert(id < id2ends_tag.size());
        if (patterns_tag[id].currentFreq == 0) {
            patterns_tag[id] = pattern;
        }
        if (addPosition) {
            assert(patterns_tag[id].currentFreq < id2ends_tag[id].size());
            id2ends_tag[id][patterns_tag[id].currentFreq] = ed;
        }
        ++ patterns_tag[id].currentFreq;
    }

    inline void addPattern(const Pattern& pattern, const TOTAL_TOKENS_TYPE& ed, bool addPosition = true) {
        assert(pattern2id.count(pattern.hashValue));
        PATTERN_ID_TYPE id = pattern2id[pattern.hashValue];
        assert(id < id2ends.size());
        separateMutex[id & SUFFIX_MASK].lock();
        if (patterns[id].currentFreq == 0) {
            patterns[id] = pattern;
        }
        if (addPosition) {
            assert(patterns[id].currentFreq < id2ends[id].size());
            id2ends[id][patterns[id].currentFreq] = ed;
        }
        ++ patterns[id].currentFreq;
        separateMutex[id & SUFFIX_MASK].unlock();
    }

    inline void addPatternPos(const Pattern& pattern, const TOTAL_TOKENS_TYPE& ed, bool addPosition = true) {
        assert(pattern2id_tag.count(pattern.hashValue));
        PATTERN_ID_TYPE id = pattern2id_tag[pattern.hashValue];
        assert(id < id2ends_tag.size());
        separateMutex[id & SUFFIX_MASK].lock();
        if (patterns_tag[id].currentFreq == 0) {
            patterns_tag[id] = pattern;
        }
        if (addPosition) {
            assert(patterns_tag[id].currentFreq < id2ends_tag[id].size());
            id2ends_tag[id][patterns_tag[id].currentFreq] = ed;
        }
        ++ patterns_tag[id].currentFreq;
        separateMutex[id & SUFFIX_MASK].unlock();
    }

    vector<bool> noExpansion, noInitial;

    inline bool pruneByPOSTag(int st, int ed) {
        if (ENABLE_POS_PRUNE) {
            POS_ID_TYPE lastPos = Documents::posTags[ed];
            if (st == ed && noInitial[lastPos] && noExpansion[lastPos]) {
                return true;
            }
            if (st != ed && noExpansion[lastPos]) {
                return true;
            }
        }
        return false;
    }

    inline void loadExternalPatterns(const string &filename, int LENGTH_THRESHOLD = 6) {
        FILE* in = tryOpen(filename, "r");
        int num_boost = 0;
        TOKEN_ID_TYPE id;
        while (getLine(in)) {
            Pattern newPattern = Pattern();
            stringstream sin(line);
            for (string temp; sin >> temp;) {
                vector<string> tokens = splitBy(temp, '_');
                assert(tokens.size() == 2);
                fromString(tokens[0], id);
                newPattern.appendwithpos(id, Documents::posTag2id[tokens[1]]);
            }
            ULL hashValue = newPattern.hashValue;
            if (!pattern2id.count(hashValue) && newPattern.size() <= LENGTH_THRESHOLD) {
                num_boost ++;
                pattern2id[hashValue] = patterns.size();
                newPattern.currentFreq = MIN_SUP;
                patterns.push_back(newPattern);
            }
        }
        cerr << id2ends.size() << "\t" <<  num_boost << endl;
    }

    inline void mine(int MIN_SUP, int LENGTH_THRESHOLD = 6) {
        // cerr << "MIN SUP is" << MIN_SUP;
        noExpansion = vector<bool>(Documents::posTag2id.size(), false);
        noInitial = vector<bool>(Documents::posTag2id.size(), false);
        if (ENABLE_POS_PRUNE) {
            FILE* in = tryOpen(NO_EXPANSION_POS_FILENAME, "r");
            int type = -1;
            while (getLine(in)) {
                if (strlen(line) == 0) {
                    continue;
                }
                stringstream sin(line);
                string tag;
                sin >> tag;
                if (tag == "===unigram===") {
                    type = 0;
                } else if (tag == "===expansion===") {
                    type = 1;
                } else {
                    if (Documents::posTag2id.count(tag)) {
                        if (type == 0) {
                            noInitial[Documents::posTag2id[tag]] = true;
                        } else if (type == 1) {
                            noExpansion[Documents::posTag2id[tag]] = true;
                        }
                    }
                }
            }
            fclose(in);

            int cntUnigrams = 0, cntExpansions = 0;
            for (int i = 0; i < noInitial.size(); ++ i) {
                cntUnigrams += noInitial[i];
                cntExpansions += noExpansion[i];
            }
            cerr << "# of forbidden initial pos tags = " << cntUnigrams << endl;
            cerr << "# of forbidden expanded pos tags = " << cntExpansions << endl;
        }

        id2ends.clear();
        patterns.clear();
        pattern2id.clear();

        unigrams = new TOTAL_TOKENS_TYPE[Documents::maxTokenID + 1];


        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (TOTAL_TOKENS_TYPE i = 0; i <= Documents::maxTokenID; ++ i) {
            unigrams[i] = 0;
        }
        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (TOTAL_TOKENS_TYPE i = 0; i < Documents::totalWordTokens; ++ i) {
            const TOTAL_TOKENS_TYPE& token = Documents::wordTokens[i];
            // add prune, see add pattern, so currentFreq must < id2ends[id].size()
            if (!pruneByPOSTag(i, i)) {
                separateMutex[token & SUFFIX_MASK].lock();
                ++ unigrams[token];
                separateMutex[token & SUFFIX_MASK].unlock();
            }
        }

        // all unigrams should be added as patterns
        // allocate memory
        for (TOTAL_TOKENS_TYPE i = 0; i <= Documents::maxTokenID; ++ i) {
            pattern2id[i + 1] = patterns.size();
            patterns.push_back(Pattern());
        }
        id2ends.resize(patterns.size());


        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (TOTAL_TOKENS_TYPE i = 0; i <= Documents::maxTokenID; ++ i) {
            id2ends[i].resize(unigrams[i] >= MIN_SUP ? unigrams[i] : 0);
        }


        long long totalOcc = 0;
        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE) reduction(+:totalOcc)
        for (TOTAL_TOKENS_TYPE i = 0; i < Documents::totalWordTokens; ++ i) {
            if (!pruneByPOSTag(i, i)) {
                const TOTAL_TOKENS_TYPE& token = Documents::wordTokens[i];
                const TOTAL_TOKENS_TYPE& postag = Documents::posTags[i];
                addPattern(Pattern(token,postag), i, unigrams[token] >= MIN_SUP);
                totalOcc += unigrams[token] >= MIN_SUP;
            }
        }

        cerr << "unigrams inserted" << endl;

        PATTERN_ID_TYPE last = 0;
        for (int len = 1; len <= LENGTH_THRESHOLD && last < patterns.size(); ++ len) {
            cerr << "# of frequent patterns of length-" << len << " = "  << patterns.size() - last + 1 << endl;
            PATTERN_ID_TYPE backup = patterns.size();

            unordered_map<ULL, TOTAL_TOKENS_TYPE> threadFreq[NTHREADS];
            # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
            for (PATTERN_ID_TYPE id = last; id < backup; ++ id) {
                assert(patterns[id].size() == 0 || patterns[id].size() == len);
                id2ends[id].shrink_to_fit();
                if (len < LENGTH_THRESHOLD) {
                    for (const TOTAL_TOKENS_TYPE& ed : id2ends[id]) {
                        TOTAL_TOKENS_TYPE st = ed - len + 1;
                        assert(Documents::wordTokens[st] == patterns[id].tokens[0]);

                        if (!Documents::isEndOfSentence(ed)) {
                        //if (!Documents::isEndOfSentence(ed) || Documents::punctuations[Documents::wordTokens[ed + 1]] == "," ) {
                            if (!pruneByPOSTag(st, ed + 1) && unigrams[Documents::wordTokens[ed + 1]] >= MIN_SUP) {
                                ULL newHashValue = patterns[id].hashValue * MAGIC + Documents::wordTokens[ed + 1] + 1;

                                int tid = omp_get_thread_num();
                                ++ threadFreq[tid][newHashValue];
                            }
                        }
                    }
                }
            }

            // merge and allocate memory
            vector<pair<ULL, TOTAL_TOKENS_TYPE>> newPatterns;
            for (int tid = 0; tid < NTHREADS; ++ tid) {
                for (const auto& iter : threadFreq[tid]) {
                    const TOTAL_TOKENS_TYPE& freq = iter.second;
                    if (freq >= MIN_SUP) {
                        const ULL& hashValue = iter.first;
                        pattern2id[hashValue] = patterns.size();
                        patterns.push_back(Pattern());
                        newPatterns.push_back(make_pair(hashValue, freq));
                    }
                }
                threadFreq[tid].clear();
            }

            id2ends.resize(patterns.size());

            # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
            for (size_t i = 0; i < newPatterns.size(); ++ i) {
                const ULL& hashValue = newPatterns[i].first;
                const TOTAL_TOKENS_TYPE& freq = newPatterns[i].second;
                id2ends[pattern2id[hashValue]].resize(freq);
            }
            newPatterns.clear();

            # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE) reduction(+:totalOcc)
            for (PATTERN_ID_TYPE id = last; id < backup; ++ id) {
                if (len < LENGTH_THRESHOLD) {
                    vector<TOTAL_TOKENS_TYPE> positions = id2ends[id];
                    for (const TOTAL_TOKENS_TYPE& ed : positions) {
                        TOTAL_TOKENS_TYPE st = ed - len + 1;
                        assert(Documents::wordTokens[st] == patterns[id].tokens[0]);

                        //if (!Documents::isEndOfSentence(ed) || Documents::punctuations[Documents::wordTokens[ed + 1]] == "," ) {
                        if (!Documents::isEndOfSentence(ed)) {
                            if (!pruneByPOSTag(st, ed + 1) && unigrams[Documents::wordTokens[ed + 1]] >= MIN_SUP) {
                                ULL newHashValue = patterns[id].hashValue * MAGIC + Documents::wordTokens[ed + 1] + 1;
                                if (pattern2id.count(newHashValue)) {
                                    Pattern newPattern(patterns[id]);
                                    //newPattern.append(Documents::wordTokens[ed + 1]);
                                    newPattern.appendwithpos(Documents::wordTokens[ed + 1], Documents::posTags[ed + 1]);
                                    assert(newPattern.size() == len + 1);
                                    newPattern.currentFreq = 0;
                                    // each ID will in different threads, so withoutlock
                                    addPatternWithoutLocks(newPattern, ed + 1);
                                    ++ totalOcc;
                                }
                            }
                        }
                    }
                    /*if (len == 1) {
                        id2ends[id].clear();
                        id2ends[id].shrink_to_fit();
                    }*/
                }
            }
            last = backup;

        }
        id2ends.shrink_to_fit();

        cerr << "# of frequent patterns = " << patterns.size() << endl;
        cerr << "total occurrence = " << totalOcc << endl;

        for (int i = 0; i < patterns.size(); ++ i) {
            assert(patterns[i].currentFreq == id2ends[i].size() || id2ends[i].size() == 0);
            assert(patterns[i].size() == 0 || patterns[i].size() == 1 || id2ends[i].size() >= MIN_SUP);
        }

        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (TOTAL_TOKENS_TYPE i = 0; i < Documents::totalWordTokens; ++ i) {
            if (pruneByPOSTag(i, i)) {
                const TOTAL_TOKENS_TYPE& token = Documents::wordTokens[i];
                addPattern(Pattern(token), i, false);
            }
        }

        // update real unigrams for later usages
        # pragma omp parallel for schedule(dynamic, PATTERN_CHUNK_SIZE)
        for (TOTAL_TOKENS_TYPE i = 0; i < Documents::totalWordTokens; ++ i) {
            const TOTAL_TOKENS_TYPE& token = Documents::wordTokens[i];
            if (pruneByPOSTag(i, i)) {
                separateMutex[token & SUFFIX_MASK].lock();
                ++ unigrams[token];
                separateMutex[token & SUFFIX_MASK].unlock();
            }
        }
    }

};

#endif
