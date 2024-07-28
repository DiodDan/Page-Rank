#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <random>
#include <omp.h>
#include <chrono>
#include <iomanip>
#include <numeric>

using namespace std;

class MonteNode {
public:
    string name;
    vector<MonteNode *> parents;
    vector<MonteNode *> children;
    uint64_t visit_count;

    explicit MonteNode(string name) {
        this->name = std::move(name);
        this->visit_count = 0;
    }

    void addParent(MonteNode *parent) {
        this->parents.push_back(parent);
    }

    void addChild(MonteNode *child) {
        this->children.push_back(child);
    }

    void normalizeVisitCount(int coeficient) {
        this->visit_count /= coeficient;
    }

    string getParents() {
        string parents_names = "";
        for (auto &parent: this->parents) {
            parents_names += parent->name + " ";
        }
        return parents_names;
    }

    string getChildren() {
        string children_names = "";
        for (auto &child: this->children) {
            children_names += child->name + " ";
        }
        return children_names;
    }
};

vector<string> splitStringMonte(const string &str, char delimiter = ' ') {
    vector<string> tokens;
    istringstream stream(str);
    string token;

    while (getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

vector<vector<string>> parseEdgesCSVMonte(const string &filePath) {
    vector<vector<string>> edges;
    ifstream file(filePath);
    string line;

    if (file.is_open()) {
        getline(file, line);  // Skipping of the header
        while (getline(file, line)) {
            istringstream iss(line);
            string segment;
            vector<string> edgePair;

            while (getline(iss, segment, '\n')) {
                edgePair = splitStringMonte(segment, ';');
                edges.push_back({edgePair[0], edgePair[1]});
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filePath << endl;
    }

    return edges;
}

void saveMonteNodesCSV(const string &filePath, const std::map<std::string, std::unique_ptr<MonteNode>> &MonteNodes) {
    ofstream file(filePath);
    file << "Id;Label;VisitCount" << endl;
    if (file.is_open()) {
        for (const auto &MonteNode: MonteNodes) {
            file << MonteNode.first << ";" << MonteNode.first << ";" << MonteNode.second->visit_count << endl;
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filePath << endl;
    }
}

std::map<std::string, std::unique_ptr<MonteNode>>
monteCarloPageRank(const std::vector<std::vector<std::string>> &edges, int num_walks, int walk_length,
                   double damping_factor) {
    std::map<std::string, std::unique_ptr<MonteNode>> MonteNodes;

    for (auto &edge: edges) {
        if (MonteNodes.find(edge[0]) == MonteNodes.end()) {
            MonteNodes[edge[0]] = std::make_unique<MonteNode>(edge[0]);
        }

        if (MonteNodes.find(edge[1]) == MonteNodes.end()) {
            MonteNodes[edge[1]] = std::make_unique<MonteNode>(edge[1]);
        }

        MonteNodes[edge[0]]->addChild(MonteNodes[edge[1]].get());
        MonteNodes[edge[1]]->addParent(MonteNodes[edge[0]].get());
    }

    vector<string> MonteNode_names;
    MonteNode_names.reserve(MonteNodes.size());
    for (auto &MonteNode: MonteNodes)
        MonteNode_names.push_back(MonteNode.first);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> MonteNode_dis(0, MonteNode_names.size() - 1);
#pragma omp parallel for default(none) shared(MonteNodes, MonteNode_names, num_walks, walk_length, damping_factor, gen, dis, MonteNode_dis)
    for (int i = 0; i < num_walks; ++i) {
        int start_MonteNode_index = MonteNode_dis(gen);
        MonteNode *current_MonteNode = MonteNodes[MonteNode_names[start_MonteNode_index]].get();

        for (int step = 0; step < walk_length; ++step) {
            current_MonteNode->visit_count++;

            if (dis(gen) < damping_factor && !current_MonteNode->children.empty()) {
                std::uniform_int_distribution<> child_dis(0, current_MonteNode->children.size() - 1);
                current_MonteNode = current_MonteNode->children[child_dis(gen)];
            } else {
                start_MonteNode_index = MonteNode_dis(gen);
                current_MonteNode = MonteNodes[MonteNode_names[start_MonteNode_index]].get();
            }
        }
    }

    return MonteNodes;
}

void benchmarkRangeThreadsMonte(const vector<vector<string>> &edges, int num_walks, int walk_length, double damping_factor,
                           int runs, int min_threads, int max_threads) {
    cout << setw(10) << "Threads" << setw(20) << "Avg Time (ms)" << endl;
    cout << string(30, '-') << endl;
    for (int threads = min_threads; threads <= max_threads; ++threads) {
        vector<double> timings;

        for (int i = 0; i < runs; ++i) {
            auto start = chrono::high_resolution_clock::now();

            omp_set_num_threads(threads);
            monteCarloPageRank(edges, num_walks, walk_length, damping_factor);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> duration = end - start;
            timings.push_back(duration.count());
        }

        double avg_time = accumulate(timings.begin(), timings.end(), 0.0) / runs;
        cout << setw(10) << threads << setw(20) << avg_time << endl;
    }
}

void benchmarkMontecarlo() {
    vector<vector<string>> edges = parseEdgesCSVMonte("../data/edges.csv");
    cout << "Starting calculations for Montecarlo" << endl;
    int num_walks = 100000;
    int walk_length = 1000;
    double damping_factor = 0.85;
    int runs = 20;
    int min_threads = 1;
    int max_threads = 12;

    benchmarkRangeThreadsMonte(edges, num_walks, walk_length, damping_factor, runs, min_threads, max_threads);
//    std::map<std::string, std::unique_ptr<MonteNode>> MonteNodes = monteCarloPageRank(edges, num_walks, walk_length, damping_factor);
//    saveMonteNodesCSV("../data/MonteNodes_Montecarlo.csv", MonteNodes);
}