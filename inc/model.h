//
// Created by carlos on 16/11/2019.
//

#ifndef MO420_BRANCH_AND_CUT_MODEL_H
#define MO420_BRANCH_AND_CUT_MODEL_H

#include "include.h"
#include "graph.h"

class Model {
    Graph *graph;
    IloEnv env;
    IloModel model;
    IloCplex cplex;
    IloArray<IloNumVarArray> x;
    IloNumVarArray y;

    int ncuts;
    float objval_relax1;
    float objval_node1;

    void objectiveFunction();

    void edgesLimitConstraint();

    void setBranchConstraint();

public:
    Model(Graph *graph);

    void initialize();

    void initModel();

    void solve();

    bool isVarInteger(IloNum x);

    ILOUSERCUTCALLBACK2(Cortes, IloArray<IloNumVarArray>, x, IloNumVarArray, y);
};

#endif //MO420_BRANCH_AND_CUT_MODEL_H
