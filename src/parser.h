#ifndef __PARSER_H__
#define __PARSER_H__

#include <cassert>
#include "../utils/utils.h"
using FrequentPatternMining::patterns;
using FrequentPatternMining::pattern2id;
namespace Parser
{
	const ULL MAGIC = 0xabcdef;
    const double INF = 1e100;
    const int maxLen = 6;

    inline ULL calcHash(const vector<TOKEN_ID_TYPE>& tokens) const {
        ULL hashvalue = 0;
        for (int i = 0; i < tokens.size(); ++i)
            hashvalue = hashvalue * MAGIC + tokens[i] + 1;
        return hashvalue;
    }

    inline void loadTestingFiles(const string& docFile) {
    	FILE* in = tryOpen(docFile, "r");
    	while (getLine(in)) {
    		//splitToToken(line, ' ')
    		segment(line);
    	}
    	fclose(in);
    }

	vector<pair<string, bool>> segment(const string &sentence) {
        vector<TOKEN_ID_TYPE> tokens = splitToToken(sentence, ' ');

    	vector<double> f(tokens.size() + 1, -INF);
    	vector<int> pre(tokens.size() + 1, -1);
    	f[0] = 0;
    	pre[0] = 0;
    	double penaltyForUnrecognizedUnigram = -1e50 / tokens.size();
    	for (size_t i = 0 ; i < tokens.size(); ++ i) {
    		if (f[i] < -1e80) {
    			continue;
    		}
    		ULL token = 0;
    		size_t j = i;
    		while (j < tokens.size()) {
    			if (j == i) {
    				//token = tokens[i];
    				token = tokens[i] + 1;
    			} else {
    				//token += " ";
    				//token += tokens[j];
    				token = token * MAGIC + tokens[i] + 1;
    			}
    			if (pattern2id.count(token)) {
    				int index = pattern2id[token];
    				double p = patterns[index].quality; 
    				if (f[i] + p > f[j + 1]) {
    					f[j + 1] = f[i] + p;
    					pre[j + 1] = i;
    				}
    			} else {
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
    	if (true) {
    	    // get the segmentation plan
    		int i = (int)tokens.size();
            vector<pair<string,bool>> segments;
    		while (i > 0) {
    			int j = pre[i];
    			string token = "";
    			for (int k = j; k < i; ++ k) {
    				if (k > j) {
    					token += " ";
    				}
    				token += tokens[k];
    			}
    			i = j;
                segments.push_back(make_pair(token, dict.count(token)));
    		}
    		reverse(segments.begin(), segments.end());
    		return segments;
    	}
    }

}

#endif