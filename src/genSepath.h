#include "utils/parameters.h"
#include "utils/utils.h"
#include "data/documents.h"

namespace GenPath
{
    int MIN_DIS;
    set<string> verb_tags = {"VB", "VBD", "VBG", "VBN", "VBP", "VBZ"};
    set<string> noun_tags = {"NN", "NNS", "NNP", "NNPS"};
    set<string> attach_tags = {"IN", "TO", "RP"};
    void split(const string &s, char delim, vector<string>& result) {
        stringstream ss;
        ss.str(s);
        string item;
        while (std::getline(ss, item, delim)) {
            result.push_back(item);
        }
    }

    void printSubtree(const vector<vector<int>>& parent, const vector<string> tags, set<int>& bgs, int index, int left, int right, bool special = false) {
        
        if (special || !noun_tags.count(tags[index - 1])) {
            for (int i = 0; i < parent[index].size(); ++i) {
                printSubtree(parent, tags, bgs, parent[index][i], left, right, special);
            }
        }
        
        if (!special && !noun_tags.count(tags[index - 1])) {
            bgs.insert(index);
        }
        if (special && attach_tags.count(tags[index - 1]))
            bgs.insert(index);
        
    }

    void printSubtree(const vector<vector<int>>& parent, const vector<int> tags, set<int>& bgs, int index, int left, int right, bool special = false) {
        
        if (special || !noun_tags.count(Documents::posid2Tag[tags[index - 1]])) {
            for (int i = 0; i < parent[index].size(); ++i) {
                printSubtree(parent, tags, bgs, parent[index][i], left, right, special);
            }
        }
        
        if (!special && !noun_tags.count(Documents::posid2Tag[tags[index - 1]])) {
            bgs.insert(index);
        }
        if (special && attach_tags.count(Documents::posid2Tag[tags[index - 1]]))
            bgs.insert(index);
        
    }

    unordered_map<int, pair<int, set<TOTAL_TOKENS_TYPE>>> genSepath(const vector<pair<int, TOTAL_TOKENS_TYPE> >& deps, const vector<int>& tags, const vector<string>& types, const vector<pair<int, int>>& entityMentions) {
    	vector<vector<int>> children(deps.size() + 1);
        vector<vector<int>> parents(deps.size() + 1);
        unordered_map<int, pair<int, set<TOTAL_TOKENS_TYPE>>> paths;
        int root;

        assert(deps.size() == types.size());
        for (int i = 0; i < deps.size(); ++ i) {
            int a = i + 1, b = deps[i].second;
            if (b == 0) {
            	children[a].push_back(a);
            }
            parents[b].push_back(a);
            int multi_root = 0;
            while (b != 0) {
                ++ multi_root;
            	children[a].push_back(b);
            	b = deps[b - 1].second;
                if (multi_root > deps.size()) {
                    return paths;
                }
        	}	
        }
        

        for (auto& item : children) {
        	reverse(item.begin(), item.end());
        }
        
        vector<vector<int>> out_nodes(entityMentions.size());
        vector<vector<string>> out_types(entityMentions.size());
        vector<string> segments;

        // cout << "here" << deps.size() << endl;

        for (int i = 0; i < entityMentions.size(); ++i) {
            // cout << entityMentions[i].first << " " << entityMentions[i].second << endl;
        	for (int index = entityMentions[i].first; index < entityMentions[i].second; ++index) {
                // cout << index << endl;
        		if (deps[index].second <= entityMentions[i].first || deps[index].second > entityMentions[i].second) {
        			if (deps[index].second == 0) {
                        /* Root node */
        				out_nodes[i].push_back(index + 1);
        			}
        			else {
        				out_nodes[i].push_back(deps[index].second);
        			}
                    out_types[i].push_back(types[index]);
        		}
        	}
        }
        
        // Shortest path version
        for (int j = 0; j < entityMentions.size(); ++j) {
            int distance = deps.size();
            int min_i = -1;
            int min_start = 0, min_end = 0, min_parent = 0;
            string start_type, end_type;
            set<int> bgs;
            for (int i = 0; i < entityMentions.size(); ++i) {
                if (i == j) continue;
                // Fix multi-out_nodes problem in this version
                for(int start_index = 0; start_index < out_nodes[i].size(); ++ start_index)
                    for (int end_index = 0; end_index < out_nodes[j].size(); ++ end_index) {
                        if (out_types[i][start_index].find("nmod") != string::npos || out_types[i][start_index].find("dobj") != string::npos || 
                           out_types[j][end_index].find("nsubj") != string::npos )
                            continue;

                //for (int start : out_nodes[i]) 
                //    for (int end : out_nodes[j]) {
                        int start = out_nodes[i][start_index];
                        int end = out_nodes[j][end_index];
                        int min_depth = min(children[start].size(), children[end].size());
                        int parent = 0, k;

                        for (k = 0; k < min_depth; ++k, parent=k) {
                            if (children[start][k] != children[end][k]) {
                                break;
                            }
                        }
                        int path_length = children[end].size() + children[start].size() + 2 - 2 * parent;
                        if (path_length <= distance) {
                            if (path_length == distance && out_types[i][start_index].find("nsubj") == string::npos && abs(i - j) >= abs(min_i - j) ) {
                                break;
                            }

                            distance = path_length;
                            min_start = start;
                            min_end = end;
                            min_parent = parent;
                            min_i = i;
                            start_type = out_types[i][start_index];
                            end_type = out_types[j][end_index];
                        }
                    }
            }

            // assert(min_parent != 0);
            if (min_parent == 0) continue;

            // cerr << min_i << " " << j << "\t" << min_start << "\t" << min_end << "\t" << min_parent << endl;
            // cerr << children[min_start].size() << "\t" << children[min_end].size() << endl;
            for (int st = min_parent; st < children[min_start].size(); ++st) {
                printSubtree(parents, tags, bgs, children[min_start][st], entityMentions[min_i].second, entityMentions[j].first);
            }

            printSubtree(parents, tags, bgs, min_start, entityMentions[min_i].second, entityMentions[j].first);
            if (min_start != min_end) {
                for (int st = min_parent; st < children[min_end].size(); ++st) {
                    printSubtree(parents, tags, bgs, children[min_end][st], entityMentions[min_i].second, entityMentions[j].first);
                }

                printSubtree(parents, tags, bgs, min_end, entityMentions[min_i].second, entityMentions[j].first);
            }
            
            
            for (int i = entityMentions[j].first; i < entityMentions[j].second; ++i) {
                printSubtree(parents, tags, bgs, i+1, entityMentions[min_i].second, entityMentions[j].first, true);
            }
            

            
            vector<int> erased;

            for (const auto& path : bgs) {
                if (path <= entityMentions[min_i].second || path > entityMentions[j].first )
                    erased.push_back(path);
            }

            for (const auto& path : erased) {
                bgs.erase(path);
            }

            // printf("%d_%s %d_%s\t", min_i, start_type.c_str(), j, end_type.c_str());
            // fprintf(out, "%d %d\t", min_i, j);

            /*
            for (const auto& t : bgs) {
                fprintf(out, "%d ", t);
            }
            */

            // fprintf(out, "<>");
            if (bgs.size() > 0) paths[j] = make_pair(min_i, bgs);
            // paths.push_back(make_pair(pair<int>(min_i), bgs));

        }
        return paths;
    }

    void process(const vector<int>& deps, const vector<string>& tags, const vector<string>& types, const vector<pair<int, int>>& entityMentions, FILE* out) {
        vector<vector<int>> children(deps.size() + 1);
        vector<vector<int>> parents(deps.size() + 1);
        int root;

        assert(deps.size() == types.size());
        for (int i = 0; i < deps.size(); ++ i) {
            int a = i + 1, b = deps[i];
            if (b == 0) {
                children[a].push_back(a);
            }
            parents[b].push_back(a);
            int multi_root = 0;
            while (b != 0) {
                ++ multi_root;
                children[a].push_back(b);
                b = deps[b - 1];
                if (multi_root > deps.size()) return;
            }   
        }
        
        /*
        for(int i = 0; i < parents.size(); ++i) {
            cerr << i << "->\t";
            for (int j = 0; j < parents[i].size(); ++j) {
                cerr << parents[i][j] << " ";
            }
            cerr << endl;

        }
        */

        for (auto& item : children) {
            reverse(item.begin(), item.end());
        }
        
        vector<vector<int>> out_nodes(entityMentions.size());
        vector<vector<string>> out_types(entityMentions.size());
        vector<string> segments;

        for (int i = 0; i < entityMentions.size(); ++i) {
            // cerr << entityMentions[i].first << " " << entityMentions[i].second << endl;
            for (int index = entityMentions[i].first; index < entityMentions[i].second; ++index) {

                if (deps[index] <= entityMentions[i].first || deps[index] > entityMentions[i].second) {
                    if (deps[index] == 0) {
                        /* Root node */
                        out_nodes[i].push_back(index + 1);
                    }
                    else {
                        out_nodes[i].push_back(deps[index]);
                    }
                    out_types[i].push_back(types[index]);
                }
            }
        }

        // Shortest path version
        for (int j = 0; j < entityMentions.size(); ++j) {
            int distance = deps.size();
            int min_i = -1;
            int min_start = 0, min_end = 0, min_parent = 0;
            string start_type, end_type;
            set<int> bgs;
            for (int i = 0; i < entityMentions.size(); ++i) {
                if (i == j) continue;
                // Fix multi-out_nodes problem in this version
                for(int start_index = 0; start_index < out_nodes[i].size(); ++ start_index)
                    for (int end_index = 0; end_index < out_nodes[j].size(); ++ end_index) {
                        if (out_types[i][start_index].find("nmod") != string::npos || out_types[i][start_index].find("dobj") != string::npos || 
                           out_types[j][end_index].find("nsubj") != string::npos )
                            continue;

                //for (int start : out_nodes[i]) 
                //    for (int end : out_nodes[j]) {
                        int start = out_nodes[i][start_index];
                        int end = out_nodes[j][end_index];
                        int min_depth = min(children[start].size(), children[end].size());
                        int parent = 0, k;

                        for (k = 0; k < min_depth; ++k, parent=k) {
                            if (children[start][k] != children[end][k]) {
                                break;
                            }
                        }
                        int path_length = children[end].size() + children[start].size() + 2 - 2 * parent;
                        if (path_length <= distance) {
                            if (path_length == distance && out_types[i][start_index].find("nsubj") == string::npos && abs(i - j) >= abs(min_i - j) ) {
                                break;
                            }

                            distance = path_length;
                            min_start = start;
                            min_end = end;
                            min_parent = parent;
                            min_i = i;
                            start_type = out_types[i][start_index];
                            end_type = out_types[j][end_index];
                        }
                    }
            }

            // assert(min_parent != 0);
            if (min_parent == 0) continue;

            // cerr << min_i << " " << j << "\t" << min_start << "\t" << min_end << "\t" << min_parent << endl;
            // cerr << children[min_start].size() << "\t" << children[min_end].size() << endl;
            for (int st = min_parent; st < children[min_start].size(); ++st) {
                printSubtree(parents, tags, bgs, children[min_start][st], entityMentions[min_i].second, entityMentions[j].first);
            }

            printSubtree(parents, tags, bgs, min_start, entityMentions[min_i].second, entityMentions[j].first);
            if (min_start != min_end) {
                for (int st = min_parent; st < children[min_end].size(); ++st) {
                    printSubtree(parents, tags, bgs, children[min_end][st], entityMentions[min_i].second, entityMentions[j].first);
                }

                printSubtree(parents, tags, bgs, min_end, entityMentions[min_i].second, entityMentions[j].first);
            }
            
            
            for (int i = entityMentions[j].first; i < entityMentions[j].second; ++i) {
                printSubtree(parents, tags, bgs, i+1, entityMentions[min_i].second, entityMentions[j].first, true);
            }
            

            
            vector<int> erased;

            for (const auto& path : bgs) {
                if (path <= entityMentions[min_i].second || path > entityMentions[j].first )
                    erased.push_back(path);
            }

            for (const auto& path : erased) {
                bgs.erase(path);
            }

            // fprintf(out, "%d_%s %d_%s\t", min_i, start_type.c_str(), j, end_type.c_str());
            fprintf(out, "%d %d\t", min_i, j);

            for (const auto& t : bgs) {
                fprintf(out, "%d ", t);
            }

            fprintf(out, "<>");

        }

    }
};