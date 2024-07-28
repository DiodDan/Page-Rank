#include <iostream>
#include <vector>
#include <string>
#include "standard.cpp"

using namespace std;


int main() {
    vector<vector<string>> edges = parseEdgesCSV("../data/edges.csv");
    cout << "Starting calculations" << endl;

    int runs = 5;
    int iterations = 100;
    int min_threads = 1;
    int max_threads = 12;

    benchmarkRangeThreads(edges, iterations, runs, min_threads, max_threads);

    return 0;
//    cout << "Starting calculations" << endl;
//    std::map<std::string, std::unique_ptr<Node>> nodes = measureExecutionTime(parallelPageRank, "Parallel", edges, 100,
//                                                                              false, 10);
//    std::map<std::string, std::unique_ptr<Node>> nodes1 = measureExecutionTime(pageRank, "Serial", edges, 100, false);
//
//    vector<string> node_names;
//    node_names.reserve(nodes.size());
//    for (auto &node: nodes)
//        node_names.push_back(node.first);
//
//    for (const auto &node_name: node_names) {
//        if (nodes[node_name]->prev_value != nodes1[node_name]->prev_value) {
//            cout << "Error in node: " << node_name << " prev_value: " << nodes[node_name]->prev_value << " != "
//                 << nodes1[node_name]->prev_value << endl;
//        }
//    }

//    saveNodesCSV("../data/nodes_w_W2.csv", nodes);

//    for (auto &node: nodes) {
//        cout << "_________" << endl << "Node name: " << node.second->name << endl << "parents : "
//             << node.second->getParents() << endl << "clildrens : " << node.second->children_number << endl
//             << "prev_value : " << node.second->prev_value << endl;
//    }

    return 0;

}