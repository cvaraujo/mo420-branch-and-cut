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

    Edge() {}

    Edge(int u, int v) {
        this->u = u;
        this->v = v;
    }
};

class Graph {
public:
    int n, m, numFixed = 0;
    vector<Edge> edges;
    vector<int> vertices;
    vector<vector<int>> incidenceMatrix;
    vector<pair<Edge, Edge>> cocycle;
    vector<Edge> bridges;
    vector<vector<bool>> isBridge;
    vector<vector<bool>> isBridgeAndCocycle;
    vector<bool> branches;
    vector<int> cutVertices;

    Graph() {
    }

    void load_graph(string path);

    void print_graph();

    void bridgeUtil(int u, bool visited[], int disc[], int low[], int parent[], bool ap[], Edge removed);

    void bridge(Edge removed);

    void twoCocycle();

    int connectedComponents(int removed);

    void DFSUtil(int v, int removed, bool visited[]);
};

#endif //MO420_BRANCH_AND_CUT_GRAPH_H
