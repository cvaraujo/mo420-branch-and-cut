//
// Created by carlos on 16/11/2019.
//

#include "../inc/graph.h"

void Graph::load_graph(string path) {
    ifstream file;
    file.open(path, fstream::in);
    file >> n >> m;

    Edge e;
    edges = vector<Edge>();
    bridges = vector<Edge>();
    vertices = vector<int>();
    branches = vector<bool>(n);

    incidenceMatrix = vector<vector<int >>(n, vector<int>());
    cocycle = vector<pair<Edge, Edge >>();
    isBridge = vector<vector<bool >>(n, vector<bool>(n));
    isBridgeAndCocycle = vector<vector<bool >>(n, vector<bool>(n));

    for (int i = 0; i < m; i++) {
        file >> e.u >> e.v;
        e.u--, e.v--;
        incidenceMatrix[e.u].push_back(e.v),
                incidenceMatrix[e.v].push_back(e.u);
        if (incidenceMatrix[e.u].size() == 3) vertices.push_back(e.u);
        if (incidenceMatrix[e.v].size() == 3) vertices.push_back(e.v);
        edges.push_back(e);
    }
    file.close();
    cout << "graph was loaded" << endl;
}

void Graph::print_graph() {
    for (auto e : edges) {
        cout << e.u << " - " << e.v << " = " << e.weight << endl;
    }
}

void Graph::set_edge_value(int u, int v, double weight) {
    Edge e;
    for (int i = 0; i < m; i++) {
        e = edges[i];
        if (e.u == u && e.v == v) {
            edges[i].weight = weight;
            break;
        }
    }
}

void Graph::bridgeUtil(int u, bool visited[], int disc[], int low[], int parent[], bool ap[], Edge removed) {
    static int time = 0;

    int children = 0;

    visited[u] = true;

    disc[u] = low[u] = ++time;

    for (auto v : incidenceMatrix[u]) {
        if ((removed.u == u && removed.v == v) || (removed.v == u && removed.u == v))
            continue;
        else {
            // If v is not visited yet, then recur for it
            if (!visited[v]) {
                children++;
                parent[v] = u;
                bridgeUtil(v, visited, disc, low, parent, ap, removed);

                // Check if the subtree rooted with v has a
                // connection to one of the ancestors of u
                low[u] = min(low[u], low[v]);

                if (parent[u] == NIL && children > 1)
                    ap[u] = true;

                if (parent[u] != NIL && low[v] >= disc[u])
                    ap[u] = true;

                if (low[v] > disc[u]) {
                    Edge e;
//                    isBridge[u][v] = isBridge[v][u] = true;
                    e.u = u, e.v = v;
                    bridges.push_back(e);
//                 cout << u << " " << v << endl;
                }

            } // Update low value of u for parent function calls.
            else if (v != parent[u])
                low[u] = min(low[u], disc[v]);
        }
    }
}

void Graph::twoCocycle() {
    Edge edge;
    edge.u = n, edge.v = n;

    bridge(edge);

    for (int u = 0; u < n; u++) {
        for (auto v : incidenceMatrix[u]) {
            edge.u = u, edge.v = v;
            if (!isBridge[u][v]) {
                bridges = vector<Edge>();
                bridge(edge);
            }
        }
    }

    for (auto p : cocycle) {
        cout << p.first.u + 1 << ", " << p.first.v + 1 << " -- " << p.second.u + 1 << ", " << p.second.v + 1 << endl;
    }

}

void Graph::bridge(Edge removed) {
    // Mark all the vertices as not visited
    bool *visited = new bool[n];
    int *disc = new int[n];
    int *low = new int[n];
    int *parent = new int[n];
    bool *ap = new bool[n];
    vector<int> countAdj = vector<int>(n);


    // Initialize parent and visited arrays
    for (int i = 0; i < n; i++) {
        parent[i] = NIL;
        visited[i] = false;
        countAdj[i] = 0;
        ap[i] = false;
    }

    // Call the recursive helper function to find Bridges
    // in DFS tree rooted with vertex 'i'
    for (int i = 0; i < n; i++)
        if (!visited[i])
            bridgeUtil(i, visited, disc, low, parent, ap, removed);

    if (removed.u == n) {
        for (auto edge : bridges) {
//            cout << edge.u + 1 << " - " << edge.v + 1 << endl;
            isBridge[edge.u][edge.v] = isBridge[edge.v][edge.u] = true;
            isBridgeAndCocycle[edge.u][edge.v] = isBridgeAndCocycle[edge.v][edge.u] = true;
            countAdj[edge.u]++, countAdj[edge.v]++;
        }

        for (int i = 0; i < n; i++) {
            if (countAdj[i] >= 2 && int(incidenceMatrix[i].size()) > 2) {
                branches[i] = true;
                numFixed++;
            }
        }
    } else
        for (auto edge : bridges) {
            if (!isBridge[edge.u][edge.v]) {
                cocycle.push_back(make_pair(removed, edge));
                isBridgeAndCocycle[edge.u][edge.v] = isBridgeAndCocycle[edge.v][edge.u] = true;
//                cout << edge.u + 1 << " - " << edge.v + 1 << endl;
            }
        }
}