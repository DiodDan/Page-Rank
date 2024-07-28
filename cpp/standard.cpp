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
#include <iomanip>
#include <numeric>

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
        string parents_names;
        for (auto &parent: this->parents) {
            parents_names += parent->name + " ";
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

#pragma omp parallel for reduction(+:sum) default(none) shared(parents)
        for (size_t i = 0; i < parents.size(); ++i) {
            sum += parents[i]->prev_value / parents[i]->children_number;
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
        nodes[node_names[node_name_index]]->setPrevValue(start_value);

#pragma omp parallel for default(none) shared(nodes, edges, node_name_index, node_names) // ask about usability
        for (int edge_index = 0; edge_index < edges.size(); edge_index++) {
            if (edges[edge_index][0] == node_names[node_name_index])
                nodes[node_names[node_name_index]]->addChild();

            if (edges[edge_index][1] == node_names[node_name_index])
                nodes[node_names[node_name_index]]->addParent(nodes[edges[edge_index][0]].get());
        }
    }


    for (int i = 0; i < iterations; i++) {
        if (verbose)
            cout << "Iteration: " << i << endl;
#pragma omp parallel for default(none) shared(nodes, node_names)
        for (int node_name_index = 0; node_name_index < node_names.size(); node_name_index++) {
            nodes[node_names[node_name_index]].get()->parallelRecalculate();
        }

#pragma omp parallel for default(none) shared(nodes, node_names)
        for (int node_name_index = 0; node_name_index < node_names.size(); node_name_index++) {
            nodes[node_names[node_name_index]].get()->step();
        }
    }
    return nodes;
}

void benchmarkParallelPageRank(const vector<vector<string>> &edges, int iterations, int runs, int threads) {
    vector<double> timings;

    for (int i = 0; i < runs; ++i) {
        auto start = chrono::high_resolution_clock::now();

        parallelPageRank(edges, iterations, false, threads);

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> duration = end - start;
        timings.push_back(duration.count());
    }

    cout << setw(10) << "Run" << setw(20) << "Execution Time (ms)" << endl;
    cout << string(30, '-') << endl;
    for (int i = 0; i < runs; ++i) {
        cout << setw(10) << i + 1 << setw(20) << timings[i] << endl;
    }
    cout << "Average execution time: " << accumulate(timings.begin(), timings.end(), 0.0) / runs << " ms" << endl;
}


void benchmarkRangeThreads(const vector<vector<string>> &edges,
                           int iterations,
                           int runs,
                           int min_threads,
                           int max_threads) {
    cout << setw(10) << "Threads" << setw(20) << "Avg Time (ms)" << endl;
    cout << string(30, '-') << endl;
    for (int threads = min_threads; threads <= max_threads; ++threads) {
        vector<double> timings;

        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();

            parallelPageRank(edges, iterations, false, threads);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> duration = end - start;
            timings.push_back(duration.count());
        }

        double avg_time = accumulate(timings.begin(), timings.end(), 0.0) / runs;
        cout << setw(10) << threads << setw(20) << avg_time << endl;
    }
}

void benchmarkStandard(){
    vector<vector<string>> edges = parseEdgesCSV("../data/edges.csv");
    cout << "Starting calculations for standart" << endl;

    int runs = 5;
    int iterations = 100;
    int min_threads = 1;
    int max_threads = 12;

    benchmarkRangeThreads(edges, iterations, runs, min_threads, max_threads);
}