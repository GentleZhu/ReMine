#ifndef __DOCUMENTS_H__
#define __DOCUMENTS_H__

#include "../utils/utils.h"

namespace Documents
{
    const int FIRST_CAPITAL = 0;
    const int ALL_CAPITAL = 1;
    const int DASH_AFTER = 2;
    const int QUOTE_BEFORE = 3;
    const int QUOTE_AFTER = 4;
    const int PARENTHESIS_BEFORE = 5;
    const int PARENTHESIS_AFTER = 6;
    const int SEPARATOR_AFTER = 7;

    struct WordTokenInfo {
        unsigned char mask;
        // 7 types:
        //    0-th bit: First Char Capital?
        //    1-st bit: All Chars Capital?
        //    2-nd bit: any - after this char?
        //    3-rd bit: any " before this char?
        //    4-th bit: any " after this char?
        //    5-th bit: any ( before this char?
        //    6-th bit: any ) after this char?
        //    7-th bit: any separator punc after this char?
        WordTokenInfo() {
            mask = 0;
        }

        inline void turnOn(int bit) {
            mask |= 1 << bit;
        }

        inline bool get(int bit) const {
            return mask >> bit & 1;
        }
    };
// === global variables ===
    TOTAL_TOKENS_TYPE totalTokens = 0;
    TOTAL_TOKENS_TYPE totalWordTokens = 0;

    TOKEN_ID_TYPE maxTokenID = 0;
    TOKEN_ID_TYPE maxPosID = 0;

    float* idf; // 0 .. maxTokenID
    TOKEN_ID_TYPE* wordTokens; // 0 .. totalWordTokens - 1

    TOKEN_ID_TYPE* depPaths;

    POS_ID_TYPE* posTags; // 0 .. totalWordTokens - 1

    // 0 .. ((totalWordTokens * 7 + 31) / 32) - 1
    WordTokenInfo* wordTokenInfo;
    bool* isDigital; // 0..maxTokenID

    vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> sentences;
    //vector<pair<TOTAL_TOKENS_TYPE, TOTAL_TOKENS_TYPE>> postagsent;

    // Below are unique pos tag information.
    map<string, POS_ID_TYPE> posTag2id;
    map<POS_ID_TYPE, string> posid2Tag;
    vector<string> posTag;

    // dependency tree maps
    unordered_map<string, double> tree_map;

    set<TOKEN_ID_TYPE> stopwords;

    set<string> separatePunc = {",", ".", "\"", ";", "!", ":", "(", ")", "\"", "\'\'"};
// ===
    inline bool hasDashAfter(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(DASH_AFTER);
    }

    inline bool hasQuoteBefore(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(QUOTE_BEFORE);
    }

    inline bool hasQuoteAfter(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(QUOTE_AFTER);
    }

    inline bool hasParentThesisBefore(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(PARENTHESIS_BEFORE);
    }

    inline bool hasParentThesisAfter(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(PARENTHESIS_AFTER);
    }

    inline bool isFirstCapital(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(FIRST_CAPITAL);
    }

    inline bool isAllCapital(int i) {
        return 0 <= i && i < totalWordTokens && wordTokenInfo[i].get(ALL_CAPITAL);
    }

    inline bool isEndOfSentence(int i) {
        return i < 0 || i + 1 >= totalWordTokens || wordTokenInfo[i].get(SEPARATOR_AFTER);
    }

    inline void loadStopwords(const string &filename) {
        FILE* in = tryOpen(filename, "r");

        while (getLine(in)) {
            vector<string> tokens = splitBy(line, ' ');
            if (tokens.size() == 1) {
                TOKEN_ID_TYPE id;
                fromString(tokens[0], id);
                if (id >= 0) {
                    stopwords.insert(id);
                }
            }
        }
        fclose(in);
    }

    inline void loadAllTrainingFiles(const string& docFile, const string& posFile, const string& capitalFile,
        const string& depFile) {
        if (true) {
            // get total number of tokens and the maximum number of tokens
            FILE* in = tryOpen(docFile, "r");
            totalTokens = 0;
            for (;fscanf(in, "%s", line) == 1; ++ totalTokens) {
                bool flag = true;
                TOKEN_ID_TYPE id = 0;
                for (int i = 0; line[i] && flag; ++ i) {
                    flag &= isdigit(line[i]);
                    id = id * 10 + line[i] - '0';
                }
                if (flag) {
                    maxTokenID = max(maxTokenID, id);
                    ++ totalWordTokens;
                }
            }
            cerr << "# of total tokens = " << totalTokens << endl;
            cerr << "# of total word tokens = " << totalWordTokens << endl;
            cerr << "max word token id = " << maxTokenID << endl;
            fclose(in);
        }

        idf = new float[maxTokenID + 1];
        isDigital = new bool[maxTokenID + 1];
        wordTokens = new TOKEN_ID_TYPE[totalWordTokens];

        if (true) {
            posTag.clear();
            posTag2id.clear();
            posid2Tag.clear();
            posTags = new POS_ID_TYPE[totalWordTokens];
        }
        wordTokenInfo = new WordTokenInfo[totalWordTokens];

        char currentTag[100];
        char currentDep[100];

        FILE* in = tryOpen(docFile, "r");
        FILE* posIn = tryOpen(posFile, "r");
        FILE* capitalIn = tryOpen(capitalFile, "r");
        FILE* depIn = NULL;
        if (ENABLE_POS_TAGGING) {
            depIn = tryOpen(depFile, "r");
            depPaths = new TOKEN_ID_TYPE[totalWordTokens];
        }
        // posIn = tryOpen(posFile, "r");

        INDEX_TYPE docs = 0;
        TOTAL_TOKENS_TYPE ptr = 0;
        while (getLine(in)) {
            ++ docs;
            TOTAL_TOKENS_TYPE docStart = ptr;

            stringstream sin(line);

            myAssert(getLine(capitalIn), "Capital info file doesn't have enough lines");
            int capitalPtr = 0;

            string lastPunc = "";
            for (string temp; sin >> temp;) {
                // get pos tag
                POS_ID_TYPE posTagId = -1;
                if (ENABLE_POS_TAGGING) {
                    myAssert(fscanf(depIn, "%s", currentDep) == 1, "Deps file doesn't have enough dependencies");
                }

                // get postag info
                myAssert(fscanf(posIn, "%s", currentTag) == 1, "POS file doesn't have enough POS tags");
                if (!posTag2id.count(currentTag)) {
                    posTagId = posTag2id.size();
                    posTag.push_back(currentTag);
                    posTag2id[currentTag] = posTagId;
                    posid2Tag[posTagId] = currentTag;
                } else {
                    posTagId = posTag2id[currentTag];
                }

                // get token
                bool flag = true;
                TOKEN_ID_TYPE token = 0;
                for (int i = 0; i < temp.size() && flag; ++ i) {
                    flag &= isdigit(temp[i]);
                    token = token * 10 + temp[i] - '0';
                }

                // get capital info
                int capitalInfo = line[capitalPtr ++];

                if (!flag) {
                    string punc = temp;
                    if (ptr > 0) {
                        if (punc == "-") {
                            wordTokenInfo[ptr - 1].turnOn(DASH_AFTER);
                        }
                        if (punc == "\"") {
                            wordTokenInfo[ptr - 1].turnOn(QUOTE_AFTER);
                        }
                        if (punc == ")" && ptr > 0) {
                            wordTokenInfo[ptr - 1].turnOn(PARENTHESIS_AFTER);
                        }
                        if (separatePunc.count(punc)) {
                            wordTokenInfo[ptr - 1].turnOn(SEPARATOR_AFTER);
                        }
                    }
                    lastPunc = punc;
                } else {
                    assert(token > 0);
                    wordTokens[ptr] = token;
                    if (ENABLE_POS_TAGGING) {
                        depPaths[ptr] = atoi(currentDep);
                    }
                    posTags[ptr] = posTagId;

                    if (lastPunc == "\"") {
                        wordTokenInfo[ptr].turnOn(QUOTE_BEFORE);
                    } else if (lastPunc == "(") {
                        wordTokenInfo[ptr].turnOn(PARENTHESIS_BEFORE);
                    }

                    if (capitalInfo & 1) {
                        wordTokenInfo[ptr].turnOn(FIRST_CAPITAL);
                    }
                    if (capitalInfo >> 1 & 1) {
                        wordTokenInfo[ptr].turnOn(ALL_CAPITAL);
                    }
                    if (capitalInfo >> 2 & 1) {
                        isDigital[token] = true;
                    }

                    ++ ptr;
                }
            }

            set<TOKEN_ID_TYPE> docSet(wordTokens + docStart, wordTokens + ptr);
            FOR (token, docSet) {
                ++ idf[*token];
            }
        }
        fclose(in);

        for (TOKEN_ID_TYPE i = 0; i <= maxTokenID; ++ i) {
            idf[i] = log(docs / idf[i] + EPS);
        }

        cerr << "# of documents = " << docs << endl;
        cerr << "# of POS tags = " << posTag2id.size() << endl;
        maxPosID = posTag2id.size() - 1 ;
    }

    inline void splitIntoSentences(bool ORIGINAL_PUNC, string docFile="") {
        sentences.clear();
        TOTAL_TOKENS_TYPE st = 0;

        if (!ORIGINAL_PUNC) {
            for (TOTAL_TOKENS_TYPE i = 0; i < totalWordTokens; ++ i) {
                if (isEndOfSentence(i)) {
                    sentences.push_back(make_pair(st, i));
                    //postagsent.push_back(make_pair(st, i));
                    st = i + 1;
                }
            }
        }
        else {
            FILE* in = tryOpen(docFile, "r");
            while (getLine(in)) {
                // line[strcspn(line, "\n")] = '\0';
                int words_cnt = 0;
                for (int i = 0; i < strlen(line); ++i) {
                    if (line[i] == ' ') words_cnt += 1;
                }
                sentences.push_back(make_pair(st, st + words_cnt - 1));
                st += words_cnt;
            }
            assert(st == totalWordTokens);
            fclose(in);
        }
        sentences.shrink_to_fit();
        cerr << "The number of sentences = " << sentences.size() << endl;
        //cout << "The number of sentences = " << sentences.size() << endl;
    }

    inline string treeToString(const vector<vector<int>>& children, int u) {
        vector<string> subtrees;
        for (int v : children[u]) {
            subtrees.push_back(treeToString(children, v));
        }
        sort(subtrees.begin(), subtrees.end());
        string ret = "(x";
        for (const string& subtree : subtrees) {
            if (u == 0)
                ret += "|"+subtree;
            else
                ret += subtree;

        }
        ret += ")";
        return ret;
    }

    inline string GetSubtreeID(const vector<int> &deps, int start, int end, unordered_map<string, double> &string_to_id) {
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
        // cerr << "Before" << endl;
        // cerr << children[0].size() << endl;
        return treeToString(children, 0);
    }

    inline double InsertOrGetSubtreeID(const vector<int> &deps, int start, int end, unordered_map<string, double> &string_to_id) {
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
        // cerr << "Before" << endl;
        // cerr << children[0].size() << endl;
        string min_representation = treeToString(children, 0);
        // cerr << "After" << endl;
        if (string_to_id.count(min_representation)) {
            return string_to_id[min_representation];
        }
        int new_id = string_to_id.size();
        if (children[0].size() == 1)
            string_to_id[min_representation] = 1;
        else
            string_to_id[min_representation] = 0.01;
        return string_to_id[min_representation];
        // return string_to_id[min_representation] = new_id;
    }

};

#endif
