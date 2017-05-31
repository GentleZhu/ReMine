#ifndef __PARSER_H__
#define __PARSER_H__

#include <cassert>
#include "../utils/utils.h"
using FrequentPatternMining::patterns;
using FrequentPatternMining::pattern2id;
class Parser
{
private:
    static const ULL MAGIC;
    static const double INF;
    static const int maxLen;
    double *prob;

public:

    inline void initialize(double* p, int size){
        prob = new double[size];
        copy(p, p+size, prob);
        //prob=p;
    }

    inline ULL calcHash(const vector<TOKEN_ID_TYPE>& tokens) {
        ULL hashvalue = 0;
        for (int i = 0; i < tokens.size(); ++i)
            hashvalue = hashvalue * MAGIC + tokens[i] + 1;
        return hashvalue;
    }

    vector<pair<string, int>> segment(const string &sentence, const unordered_set<int>& dict) {
        vector<TOKEN_ID_TYPE> tokens = splitToToken(sentence, ' ');
        //vector<string> tokenss = splitBy(sentence, ' ');
        vector<double> f(tokens.size() + 1, -INF);
        vector<int> pre(tokens.size() + 1, -1);
        f[0] = 0;
        pre[0] = 0;
        int Matched=0;
        double penaltyForUnrecognizedUnigram = -1e50 / tokens.size();
        //fstream fout;
        //fout.open("tmp/matched_token.txt", fstream::out);
        FILE* out = tryOpen("tmp/matched_token.txt", "w");
        for (size_t i = 0 ; i < tokens.size(); ++ i) {
            if (f[i] < -1e80) {
                continue;
            }
            ULL token = 0;
            string t = "";
            size_t j = i;
            while (j < tokens.size()) {
                if (j == i) {
                    //t = to_string(tokens[i]);
                    //token = tokens[i];
                    token = tokens[i] + 1;
                } else {
                    //t += " ";
                    //t += to_string(tokens[j]);
                    //cerr<<t<<endl;
                    token = token * MAGIC + tokens[j] + 1;
                }
                if (pattern2id.count(token)) {
                    
                    //cout << "Matched: " << t << endl;
                    //cout << "Matched" <<endl;
                    int index = pattern2id[token];
                    double p = prob[index];
                    fprintf(out, "%lf ", p);
                    for (int tt = 0; tt < patterns[index].tokens.size(); ++ tt) {

                        fprintf(out, "%d%c", patterns[index].tokens[tt], tt + 1 == patterns[index].tokens.size() ? '\n' : ' ');
                    }
                    //cerr<<p<<endl;
                    //if (j>i)
                    //    cerr<<t<<endl;
                        //fout<<t<<" "<<p<<endl;
                        //fprintf(out, "%s\t%.10f\n", t,p);
                        //cerr<<p<<endl;
                    if (f[i] + p > f[j + 1]) {
                        f[j + 1] = f[i] + p;
                        pre[j + 1] = i;
                    }
                } else {
                    //cout << "Unmatched: " << t << endl;
                    //cout << "Unmatched" <<endl;
                    if (i == j) {
                        double p = penaltyForUnrecognizedUnigram;
                        if (f[i] + p > f[j + 1]) {
                            f[j + 1] = f[i] + p;
                            pre[j + 1] = i;
                        }
                    }
                    if (j > maxLen + i) {
                        break;
                    }
                }
                ++ j;
            }
        }
        //fout.close();
        //cerr<<"Matched: "<<Matched<<endl;
        if (true) {
            // get the segmentation plan
            int i = (int)tokens.size();
            //vector<pair<string,bool>> segments;
            vector<pair<string, int>> segments;
            while (i > 0) {
                int j = pre[i];
                ULL token = 0;
                string t = "";
                for (int k = j; k < i; ++ k) {
                    if (k > j) {
                        t += " ";
                    }
                    token = token * MAGIC + tokens[k] + 1;
                    t += to_string(tokens[k]);
                }
                i = j;
                if (dict.count(pattern2id[token]))
                    segments.push_back(make_pair(t,pattern2id[token]));
            }
            reverse(segments.begin(), segments.end());
            return segments;
        }

    }

    inline vector<vector<pair<string, int>>> loadTestingFiles(const string& docFile, const unordered_set<int>& dict) {
    	FILE* in = tryOpen(docFile, "r");
        vector<vector<pair<string, int>>> tmp;
    	while (getLine(in)) {
    		//splitToToken(line, ' ')
    		tmp.push_back(segment(line, dict));
    	}
    	fclose(in);
        return tmp;
    }

	

};

const ULL Parser::MAGIC = 0xabcdef;
const double Parser::INF = 1e100;
const int Parser::maxLen = 6;
#endif