//
// Created by carlos on 16/11/2019.
//

#ifndef MO420_BRANCH_AND_CUT_MODEL_H
#define MO420_BRANCH_AND_CUT_MODEL_H

#include "include.h"
#include "graph.h"

class Model {
    IloEnv env;
    IloModel model;
    IloArray<IloNumVarArray> x, z;
    IloNumVarArray y;

    void objectiveFunction();

    void edgesLimitConstraint();

    void setBranchConstraint();

    void branchConstraintAdpt();

public:
    Graph *graph;
    int qtd_sec;
    int qtd_18;
    int qtd_19;
    int qtd_34;
    IloCplex cplex;

    float relax1;
    float obj1;

    Model(Graph *graph);

    void initialize();

    void initializeHybrid();

    void initModelHybrid();

    void outDegreeHybrid(int root);

    void branchesHybrid(int root);

    void branchesCorrelationHybrid(int root);

    void xAndZRelation();

    void initModel();

    void solve(int r18, int r19, int r34, int heuristic);

    void showSolution();

    void showSolutionHybrid();

};

#endif //MO420_BRANCH_AND_CUT_MODEL_H
