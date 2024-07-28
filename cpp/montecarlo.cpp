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

class Node {
public:
    string name;
    vector<Node *> parents;
    vector<Node *> children;
    uint64_t visit_count;

    explicit Node(string name) {
        this->name = std::move(name);
        this->visit_count = 0;
    }

    void addParent(Node *parent) {
        this->parents.push_back(parent);
    }

    void addChild(Node *child) {
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
        getline(file, line);  // Пропустить заголовок
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
    file << "Id;Label;VisitCount" << endl;
    if (file.is_open()) {
        for (const auto &node: nodes) {
            file << node.first << ";" << node.first << ";" << node.second->visit_count << endl;
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filePath << endl;
    }
}

std::map<std::string, std::unique_ptr<Node>>
monteCarloPageRank(const std::vector<std::vector<std::string>> &edges, int num_walks, int walk_length,
                   double damping_factor) {
    std::map<std::string, std::unique_ptr<Node>> nodes;

    for (auto &edge: edges) {
        if (nodes.find(edge[0]) == nodes.end()) {
            nodes[edge[0]] = std::make_unique<Node>(edge[0]);
        }

        if (nodes.find(edge[1]) == nodes.end()) {
            nodes[edge[1]] = std::make_unique<Node>(edge[1]);
        }

        nodes[edge[0]]->addChild(nodes[edge[1]].get());
        nodes[edge[1]]->addParent(nodes[edge[0]].get());
    }

    vector<string> node_names;
    node_names.reserve(nodes.size());
    for (auto &node: nodes)
        node_names.push_back(node.first);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> node_dis(0, node_names.size() - 1);
#pragma omp parallel for default(none) shared(nodes, node_names, num_walks, walk_length, damping_factor, gen, dis, node_dis)
    for (int i = 0; i < num_walks; ++i) {
        int start_node_index = node_dis(gen);
        Node *current_node = nodes[node_names[start_node_index]].get();

        for (int step = 0; step < walk_length; ++step) {
            current_node->visit_count++;

            if (dis(gen) < damping_factor && !current_node->children.empty()) {
                std::uniform_int_distribution<> child_dis(0, current_node->children.size() - 1);
                current_node = current_node->children[child_dis(gen)];
            } else {
                start_node_index = node_dis(gen);
                current_node = nodes[node_names[start_node_index]].get();
            }
        }
    }

    return nodes;
}

void benchmarkRangeThreads(const vector<vector<string>> &edges, int num_walks, int walk_length, double damping_factor,
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

int main() {
    vector<vector<string>> edges = parseEdgesCSV("../data/edges.csv");
    cout << "Starting calculations" << endl;
    int num_walks = 100000;
    int walk_length = 1000;
    double damping_factor = 0.85;
    int runs = 20;
    int min_threads = 1;
    int max_threads = 16;
//
//    benchmarkRangeThreads(edges, num_walks, walk_length, damping_factor, runs, min_threads, max_threads);
    std::map<std::string, std::unique_ptr<Node>> nodes = monteCarloPageRank(edges, num_walks, walk_length, damping_factor);
    saveNodesCSV("../data/nodes_Montecarlo.csv", nodes);
    return 0;
}