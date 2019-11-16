//
// Created by carlos on 16/11/2019.
//

#ifndef MO420_BRANCH_AND_CUT_GRAPH_H
#define MO420_BRANCH_AND_CUT_GRAPH_H

#include "include.h"

using namespace std;

class Edge {
public:
    int u;
    int v;
    double weight = 0;

    Edge() {}

    Edge(int u, int v, double weight) {
        this->u = u;
        this->v = v;
        this->weight = weight;
    }

    bool operator<(const Edge &Edge) const {
        return (this->weight < Edge.weight);
    }
};

class Graph {
public:
    int n, m, numFixed = 0;
    vector<Edge> edges;
    vector<int> vertices;
    vector<vector<int>> incidenceMatrix;
    vector<Edge> bridges;
    vector<vector<bool>> isBridge;
    vector<bool> fixed;

    Graph() {
    }

    void load_graph(string path);

    void print_graph();

    void set_edge_value(int u, int v, double weight);

    void bridgeUtil(int u, bool visited[], int disc[], int low[], int parent[]);

    void bridge();
};

#endif //MO420_BRANCH_AND_CUT_GRAPH_H
