#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>

using namespace std;


class Node {
public:
    string name;
    long double prev_value;
    long double real_value;
    vector<Node *> parents;
    uint32_t children_number;

    explicit Node(string name) {
        this->name = std::move(name);
        this->prev_value = 0;
        this->real_value = 0;
        this->children_number = 0;
    }

    void addParent(Node *parent) {
        this->parents.push_back(parent);
    }

    string getParents() {
        string parents_names = "";
        for (int i = 0; i < this->parents.size(); i++) {
            parents_names += this->parents[i]->name + " ";
        }
        return parents_names;
    }

    void setPrevValue(long double value) {
        this->prev_value = value;
    }

    void addChild() {
        this->children_number++;
    }

    void recalculate() {
        long double sum = 0;
        for (auto &parent: this->parents) {
            sum += parent->prev_value / parent->children_number;
        }
        this->real_value = sum;
    }

    void step() {
        this->prev_value = this->real_value;
        this->real_value = 0;
    }
};

vector<string> splitString(const string &str, char delimiter = ' ') {
    vector<string> tokens;
    istringstream stream(str);
    string token;

    while (getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

vector<vector<string>> parseEdgesCSV(const string &filePath) {
    vector<vector<string>> edges;
    ifstream file(filePath);
    string line;

    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            istringstream iss(line);
            string segment;
            vector<string> edgePair;

            while (getline(iss, segment, '\n')) {
                edgePair = splitString(segment, ';');
                edges.push_back({edgePair[0], edgePair[1]});
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filePath << endl;
    }

    return edges;
}

int main() {
    vector<vector<string>> edges = parseEdgesCSV("../data/test.csv");

    std::map<std::string, std::unique_ptr<Node>> nodes;

    uint64_t nodes_count = 0;


    for (auto &edge: edges) {
        if (nodes.find(edge[0]) == nodes.end()) {
            nodes[edge[0]] = std::make_unique<Node>(edge[0]);
            nodes_count++;
        }

        if (nodes.find(edge[1]) == nodes.end()) {
            nodes[edge[1]] = std::make_unique<Node>(edge[1]);
            nodes_count++;
        }
    }
    long double prev_value = 1.0 / (float) nodes_count;

    for (auto &node: nodes) {
        node.second->setPrevValue(prev_value);
        for (auto &edge: edges) {
            if (edge[0] == node.first) {
                node.second->addChild();
            }
            if (edge[1] == node.first) {
                node.second->addParent(nodes[edge[0]].get());
            }
        }
    }
    for(int i = 0; i < 100; i++) {
        cout << "Iteration: " << i << endl;
        for (auto &node: nodes) {
            node.second->recalculate();
        }

        for (auto &node: nodes) {
            node.second->step();
        }
    }

    for (auto &node: nodes) {
        cout << "_________"  << endl << "Node name: " << node.second->name << endl << "parents : " << node.second->getParents() << endl << "clildrens : " << node.second->children_number << endl << "prev_value : " << node.second->prev_value << endl;
    }

    return 0;

}