#include <fstream>
#include "utils/parameters.h"
#include "utils/utils.h"
void split(const string &s, char delim, vector<string>& result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
}

void printSubtree(const vector<vector<int>>& parent, set<int>& bgs, int index) {
    for (int i = 0; i < parent[index].size(); ++i)
        bgs.insert(parent[index][i]);
    bgs.insert(index);
}

void process(const vector<int>& deps, const vector<pair<int, int>>& entityMentions) {
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
    	for (int index = entityMentions[i].first; index < entityMentions[i].second; ++index) {
    		if (deps[index] <= entityMentions[i].first || deps[index] > entityMentions[i].second) {
    			if (deps[index] == 0) {
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
            cout << i << " " << j << "\t";
    		if (out_nodes[i].size() == 1 && out_nodes[j].size() == 1) {
    			int start = out_nodes[i][0];
    			int end = out_nodes[j][0];
    			
    			int min_depth = min(children[start].size(), children[end].size());
    			int parent = 0;
    			// if access root, 
    			for (int i = 0; i < min_depth; ++i, parent=i) {
    				if (children[start][i] != children[end][i]) {	
    					break;
    				}
    			}
                //cerr << "start" << endl;
    			// cerr << "start" << start << "end" << end <<" parent" << parent <<endl;
                if (parent == 0)
                    cerr << children[start][0] << children[end][0] << endl;
    			for (int st = parent; st < children[start].size(); ++st) {
                    printSubtree(parents, bgs, children[start][st]);
                    printSubtree(parents, bgs, start);
                }
					// segments.back() += to_string(children[start][st]) + " ";
				// segments.back() += "|";
				// segments.back() += to_string(children[start][parent-1]);
				// segments.back() += "|";
                // cerr << "no" << endl;
				for (int st = parent; st < children[end].size(); ++st) {
					// segments.back() += " " + to_string(children[end][st]);
                    printSubtree(parents, bgs, children[end][st]);
                    printSubtree(parents, bgs, end);
                }
                // cerr << "end" << endl;
                
                for (int ele = entityMentions[i].first; ele < entityMentions[i].second; ++ele)
                    bgs.erase(ele + 1);
                for (int ele = entityMentions[j].first; ele < entityMentions[j].second; ++ele)
                    bgs.erase(ele + 1);

                for (const auto& t : bgs)
                    cout << t << " ";
    		}
            cout << "<>";
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

	FILE* emIn = tryOpen(argv[2], "r");
	int docs = 0;
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
		process(depPaths[docs], ems);
        cout << endl;
        ++ docs;
        //if (docs == 5)
		//  break;
	}

}
