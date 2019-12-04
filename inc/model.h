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
    IloArray<IloNumVarArray> x, z;
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

    void initializeHybrid();

    void initModelHybrid();

    void edgesLimitConstraintHybrid();

    void outDegreeHybrid(int root);

    void branchesHybrid(int root);

    void branchesCorrelationHybrid(int root);

    void initModel();

    void solve();

    void showSolution();

    bool isVarInteger(IloNum x);

//    ILOUSERCUTCALLBACK(Cortes, IloArray<IloNumVarArray>, x, IloNumVarArray, y);
};

#endif //MO420_BRANCH_AND_CUT_MODEL_H
