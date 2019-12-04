//
// Created by carlos on 16/11/2019.
//

#include "../inc/model.h"

Model::Model(Graph *graph) {
    if (graph != nullptr) {
        this->graph = graph;

        initialize();
    }
}

void Model::initialize() {
    try {
        char name[20];
        this->model = IloModel(env);
        this->cplex = IloCplex(model);

        this->x = IloArray<IloNumVarArray>(env, graph->n);
        this->y = IloNumVarArray(env, graph->n);

        for (int i = 0; i < graph->n; i++)
            this->x[i] = IloNumVarArray(env, graph->n);


        for (auto i : graph->vertices) {
            sprintf(name, "y_%d", i);
            y[i] = IloNumVar(env, 0, 1, name);
            model.add(y[i]);
            model.add(IloConversion(env, y[i], ILOBOOL));
            for (auto j : graph->incidenceMatrix[i]) {
                sprintf(name, "x_%d_%d", i, j);
                x[i][j] = IloNumVar(env, 0, 1, name);
                model.add(x[i][j]);
                model.add(IloConversion(env, x[i][j], ILOBOOL));
            }
        }
    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
        exit(EXIT_FAILURE);
    }

}

void Model::initModel() {
    cout << "Begin the model creation" << endl;
    cplex.setParam(IloCplex::Param::TimeLimit, 600);
    cplex.setParam(IloCplex::TreLim, 7000);
//    cplex.setOut(env.getNullStream());

    objectiveFunction();
    edgesLimitConstraint();
    setBranchConstraint();
    cout << "All done!" << endl;
}

void Model::objectiveFunction() {
    IloExpr objExpr(env);
    for (auto i : graph->vertices) objExpr += y[i];
    IloObjective obj = IloObjective(env, objExpr, IloObjective::Minimize);
    model.add(obj);
    cout << "Objective Function was added successfully!" << endl;
}

void Model::edgesLimitConstraint() {
    IloExpr constraint(env);

    for (auto i : graph->vertices)
        for (auto j : graph->incidenceMatrix[i])
            constraint += x[i][j];

    model.add(constraint == (graph->n - 1));
}

void Model::setBranchConstraint() {
    IloExpr constraint(env);

    for (auto i : graph->vertices) {
        for (auto j : graph->incidenceMatrix[i]) {
            constraint += x[i][j];
        }
        model.add(constraint - 2 <= (int(graph->incidenceMatrix.size()) - 2) * y[i]);
    }
}

void Model::solve() {
    this->cplex.exportModel("model.lp");
    this->cplex.solve();
}