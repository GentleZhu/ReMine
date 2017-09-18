/*
Usage:
./bin/remine_baseline ./data_remine/nyt_deps_train.txt remine_extraction/ver2/nyt_6k_remine_2.txt remine_extraction/ver2/nyt_6k_remine_pos.txt remine_extraction/ver2/nyt_6k_remine_4.txt
*/

#include <fstream>
#include "utils/parameters.h"
#include "utils/utils.h"

int MIN_DIS = 0;
set<string> verb_tags = {"VB", "VBD", "VBG", "VBN", "VBP", "VBZ"};
void split(const string &s, char delim, vector<string>& result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
}

void printSubtree(const vector<vector<int>>& parent, const vector<string> tags, set<int>& bgs, int index) {
    if (verb_tags.count(tags[index - 1])) {
        for (int i = 0; i < parent[index].size(); ++i)
            bgs.insert(parent[index][i]);
    }
    bgs.insert(index);
}

void process(const vector<int>& deps, const vector<string>& tags, const vector<pair<int, int>>& entityMentions, FILE* out) {
	vector<vector<int>> children(deps.size() + 1);
    vector<vector<int>> parents(deps.size() + 1);
    int root;
    for (int i = 0; i < deps.size(); ++ i) {
        int a = i + 1, b = deps[i];
        if (b == 0) {
        	children[a].push_back(a);
        }
        parents[b].push_back(a);
        while (b != 0) {
        	children[a].push_back(b);
        	b = deps[b - 1];
    	}	
    }

    for (auto& item : children) {
    	reverse(item.begin(), item.end());
    }
    
    /*
    for (int i = 0; i < children.size(); ++i) {
    	 cout << "node : " << i << " ";
    	for (const auto& t : children[i])
    		cout << t << " ";
        cout << "childrens: ";
        for (const auto& t : parents[i])
            cout << t << " ";
    	cout << endl;
    }
    */
    

    vector<vector<int>> out_nodes(entityMentions.size());
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
    		}
    	}
    }

    for (int i = 0; i < entityMentions.size(); ++i) {
    	for (int j = i + 1; j < entityMentions.size(); ++j) {
    		set<int> bgs;
            // cerr << i << " " << j << "\t";
    		if (out_nodes[i].size() == 1 && out_nodes[j].size() == 1) {
    			int start = out_nodes[i][0];
    			int end = out_nodes[j][0];
    			
    			int min_depth = min(children[start].size(), children[end].size());
    			int parent = 0, k;
    			// if access root,
                bool connect = true; 
                //cout << "min depth:" << min_depth << endl;
    			for (k = 0; k < min_depth; ++k, parent=k) {
    				if (children[start][k] != children[end][k]) {
                        if (verb_tags.count(tags[children[start][k] - 1]) && verb_tags.count(tags[children[end][k] - 1]))
                            connect = false;
                        //cout << children[end].size() + children[start].size() - 2*k << endl;
    					break;
    				}
    			}

                // cerr << "parent" << parent << endl;
                // cerr << children[end].size() << "\t" << children[start].size() << endl;
                if (children[end].size() + children[start].size() + 2 - 2*parent > MIN_DIS)
                    connect = false;

                if (!connect) continue;

                //cerr << "start" << endl;
    			//cerr << "start" << start << "end" << end <<" parent" << parent << endl;
                //cerr << children[start].size() << " " << children[end].size() << endl;

                // start from root
                if (parent == 0)
                    cerr << children[start][0] << children[end][0] << endl;
    			for (int st = parent; st < children[start].size(); ++st) {
                    printSubtree(parents, tags, bgs, children[start][st]);
                    //printSubtree(parents, bgs, start);
                }

                printSubtree(parents, tags, bgs, start);
                //cerr << "start phrase size:" << bgs.size() << endl;
					// segments.back() += to_string(children[start][st]) + " ";
				// segments.back() += "|";
				// segments.back() += to_string(children[start][parent-1]);
				// segments.back() += "|";
                // cerr << "no" << endl;
				for (int st = parent; st < children[end].size(); ++st) {
					// segments.back() += " " + to_string(children[end][st]);
                    printSubtree(parents, tags, bgs, children[end][st]);
                    //printSubtree(parents, bgs, end);
                }   
                printSubtree(parents, tags, bgs, end);
                //cerr << "end phrase size:" << bgs.size() << endl;
                // cerr << "end" << endl;
                
                vector<int> erased;

                for (const auto& path : bgs) {
                    if (path <= entityMentions[i].second || path > entityMentions[j].first )
                        erased.push_back(path);
                }

                for (const auto& path : erased) {
                    bgs.erase(path);
                }
                fprintf(out, "%d %d\t", i, j);
                for (const auto& t : bgs) {
                    // cout << t << " ";
                    fprintf(out, "%d ", t);
                    // fprintf(out_dep, "%d\n", deps[t - 1]);
                }
    		}
            fprintf(out, "<>");
            // cout << "<>";
    	}
    }

    /*
    for (const auto& seg : segments) {
    	if (seg.length() > 0) {
    		cout << seg << endl;
    	}
    }
    */

}

int main(int argc, char* argv[])
{
	FILE* depIn = tryOpen(argv[1], "r");
	vector<vector<int>> depPaths;
	while (getLine(depIn)) {
		vector<int> tmp;
		depPaths.push_back(tmp);
		stringstream sin(line);
		for(string temp; sin >> temp;) {
			depPaths.back().push_back(stoi(temp));
			//cout << temp << " ";
		}
	}
    fclose(depIn);
    vector<vector<string>> posPaths;
    FILE* posIn = tryOpen(argv[3], "r");
    while (getLine(posIn)) {
        vector<string> tmp;
        posPaths.push_back(tmp);
        stringstream sin(line);
        for (string temp; sin >> temp;) {
            posPaths.back().push_back(temp);
        }

    }

	FILE* emIn = tryOpen(argv[2], "r");

    MIN_DIS = 4;
	int docs = 0;
    FILE* out = tryOpen(argv[4], "w");
    // FILE* out_dep = tryOpen(argv[5], "w");

	while (getLine(emIn)) {
        // cerr << docs << "DOC" << endl;
		stringstream sin(line);
		vector<pair<int ,int>> ems;
		for(string temp; sin >> temp;) {
			vector<string> segs;
			split(temp, '_', segs);
			assert(segs.size() == 2);
			ems.push_back(make_pair(stoi(segs[0]), stoi(segs[1])));
		}
        process(depPaths[docs], posPaths[docs], ems, out);
        fprintf(out, "\n");
        //cout << endl;
        ++ docs;
        // break;
        //if (docs == 5)
		//  break;
	}
    fclose(emIn);
    fclose(out);

}
