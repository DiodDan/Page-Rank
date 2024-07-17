#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <omp.h>

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

    void parallelRecalculate() {
        long double sum = 0;
        for (auto &parent: this->parents) {
#pragma omp critical
            {
                sum += parent->prev_value / parent->children_number;
            }
        }
        this->real_value = sum;
    }

    void step() {
        this->prev_value = this->real_value;
        this->real_value = 0;
    }
};


template<typename F, typename... Args>
auto measureExecutionTime(F func, string comment, Args &&... args) -> decltype(func(args...)) {
    auto start = std::chrono::high_resolution_clock::now();

    auto result = func(std::forward<Args>(args)...);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "Execution time for " + comment + " : " << duration.count() << " milliseconds" << std::endl;

    return result;
}

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

void saveNodesCSV(const string &filePath, const std::map<std::string, std::unique_ptr<Node>> &nodes) {
    ofstream file(filePath);
    file << "Id;Label;Weight" << endl;
    if (file.is_open()) {
        for (const auto &node: nodes) {
            file << node.first << ";" << node.first << ";" << node.second->prev_value << endl;
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filePath << endl;
    }
}


std::map<std::string, std::unique_ptr<Node>>
pageRank(const std::vector<std::vector<std::string>> &edges, int iterations = 100, bool verbose = false) {
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

    for (int i = 0; i < iterations; i++) {
        if (verbose)
            cout << "Iteration: " << i << endl;
        for (auto &node: nodes) {
            node.second->recalculate();
        }

        for (auto &node: nodes) {
            node.second->step();
        }
    }
    return nodes;
}

std::map<std::string, std::unique_ptr<Node>>
parallelPageRank(const std::vector<std::vector<std::string>> &edges,
                 int iterations = 100,
                 bool verbose = false,
                 int threads = 4) {
    omp_set_num_threads(threads);
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
    long double start_value = 1.0 / (float) nodes_count;

    vector<string> node_names;
    node_names.reserve(nodes.size());
    for (auto &node: nodes)
        node_names.push_back(node.first);

#pragma omp parallel for default(none) shared(nodes, node_names, edges, start_value)
    for (int node_name_index = 0; node_name_index < node_names.size(); node_name_index++) {
        nodes[node_names[node_name_index]].get()->setPrevValue(start_value);
#pragma omp parallel for default(none) shared(nodes, edges, node_name_index, node_names) // ask about usability
        for (int edge_index = 0; edge_index < edges.size(); edge_index++) {
            if (edges[edge_index][0] == node_names[node_name_index])
                nodes[node_names[node_name_index]].get()->addChild();

            if (edges[edge_index][1] == node_names[node_name_index])
                nodes[node_names[node_name_index]].get()->addParent(nodes[edges[edge_index][0]].get());
        }
    }


    for (int i = 0; i < iterations; i++) {
        if (verbose)
            cout << "Iteration: " << i << endl;
#pragma omp parallel for default(none) shared(nodes, node_names)
        for (int node_name_index = 0; node_name_index < node_names.size(); node_name_index++) {
            nodes[node_names[node_name_index]].get()->recalculate();
        }

#pragma omp parallel for default(none) shared(nodes, node_names)
        for (int node_name_index = 0; node_name_index < node_names.size(); node_name_index++) {
            nodes[node_names[node_name_index]].get()->step();
        }
    }
    return nodes;
}


int main() {
    vector<vector<string>> edges = parseEdgesCSV("../data/edges.csv");
    cout << "Starting calculations" << endl;
    std::map<std::string, std::unique_ptr<Node>> nodes = measureExecutionTime(parallelPageRank, "Parallel", edges, 100,
                                                                              false, 10);
    std::map<std::string, std::unique_ptr<Node>> nodes1 = measureExecutionTime(pageRank, "Serial", edges, 100, false);

    vector<string> node_names;
    node_names.reserve(nodes.size());
    for (auto &node: nodes)
        node_names.push_back(node.first);

    for (const auto &node_name: node_names) {
        if (nodes[node_name]->prev_value != nodes1[node_name]->prev_value) {
            cout << "Error in node: " << node_name << " prev_value: " << nodes[node_name]->prev_value << " != "
                 << nodes1[node_name]->prev_value << endl;
        }
    }

//    saveNodesCSV("../data/nodes_w_W2.csv", nodes);

//    for (auto &node: nodes) {
//        cout << "_________" << endl << "Node name: " << node.second->name << endl << "parents : "
//             << node.second->getParents() << endl << "clildrens : " << node.second->children_number << endl
//             << "prev_value : " << node.second->prev_value << endl;
//    }

    return 0;

}