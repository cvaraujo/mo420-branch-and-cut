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


        for (int i = 0; i < graph->n; i++) {
            sprintf(name, "y%d", i);
            y[i] = IloNumVar(env, 0, 1, name);
            model.add(y[i]);
            model.add(IloConversion(env, y[i], ILOBOOL));
            for (auto j : graph->incidenceMatrix[i]) {
                if (i < j) {
                    sprintf(name, "x%d%d", i, j);
                    x[i][j] = IloNumVar(env, 0, 1, name);
                    model.add(x[i][j]);
                    model.add(IloConversion(env, x[i][j], ILOBOOL));
                }
            }
        }
    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
        exit(EXIT_FAILURE);
    }

}

void Model::initModel() {
    cout << "Begin the model creation" << endl;
    cplex.setParam(IloCplex::Param::TimeLimit, 100);
    cplex.setParam(IloCplex::TreLim, 7000);
//    cplex.setOut(env.getNullStream());

    objectiveFunction();
    edgesLimitConstraint();
    setBranchConstraint();
    cout << "All done!" << endl;
}

void Model::objectiveFunction() {
    IloExpr objExpr(env);
    for (int i = 0; i < graph->n; i++) objExpr += y[i];
    IloObjective obj = IloObjective(env, objExpr, IloObjective::Minimize);
    model.add(obj);
    cout << "Objective Function was added successfully!" << endl;
}

void Model::edgesLimitConstraint() {
    IloExpr constraint(env);

    for (int i = 0; i < graph->n; i++)
        for (auto j : graph->incidenceMatrix[i])
            if (i < j) constraint += x[i][j];
    model.add(constraint == (graph->n - 1));

    // Edges in a extreme vertex with degree one have to be one
//    for (int i = 0; i < graph->n; i++)
//        for (auto j : graph->incidenceMatrix[i])
//            if (int(graph->incidenceMatrix[j].size()) == 1)
//                model.add(x[i][j] == 1);

}

void Model::setBranchConstraint() {
    for (int i = 0; i < graph->n; i++) {
        IloExpr constraint(env);
        for (auto j : graph->incidenceMatrix[i]) {
            constraint += x[min(i,j)][max(i,j)];
        }
        model.add((constraint - 2) <= (int(graph->incidenceMatrix[i].size()) - 2) * y[i]);
    }
/**
    // Each vertex with degree less or equal to 2 cannot be a branche
    for (int i = 0; i < graph->n; i++)
        if (int(graph->incidenceMatrix[i].size()) <= 2) model.add(y[i] == 0);

    // Vertex with two or more bridges adjacent should be a branche and cut vertex with result in 3 or more CC
    for (int i = 0; i < graph->n; i++)
        if (graph->branches[i]) model.add(y[i] == 1);

    // Cocycle restriction
    for (auto p : graph->cocycle)
        model.add(x[p.first.u][p.first.v] + x[p.second.u][p.second.v] >= 1);
**/
}

void Model::solve() {
    this->cplex.exportModel("model.lp");
    this->cplex.solve();
}

void Model::showSolution() {
    try {
        cout << "Objective" << endl;
        cout << cplex.getObjValue() << endl;

        cout << "Selected edges" << endl;
        for (int i = 0; i < graph->n; i++) {
            for (auto j : graph->incidenceMatrix[i]) {
                if (i < j && cplex.getValue(x[i][j]) > 0.5) {
                    cout << "[" << i + 1 << ", " << j + 1 << "]" << endl;
                }
            }
        }

        cout << "Branch vertex" << endl;
        for (int i = 0; i < graph->n; i++)
//            if (c >= 0.5)
            cout << "[" << i + 1 << ", " << cplex.getValue(y[i]) << "]" << endl;

    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
    }
}

/*-------------------------------------------------------

void Model::initializeHybrid() {
    try {
        char name[20];
        this->model = IloModel(env);
        this->cplex = IloCplex(model);

        this->z = IloArray<IloNumVarArray>(env, graph->n);
        this->y = IloNumVarArray(env, graph->n);

        for (int i = 0; i < graph->n; i++)
            this->x[i] = IloNumVarArray(env, graph->n);


        for (int i = 0; i < graph->n; i++) {
            sprintf(name, "y%d", i);
            y[i] = IloNumVar(env, 0, 1, name);
            model.add(y[i]);
            model.add(IloConversion(env, y[i], ILOBOOL));
            for (auto j : graph->incidenceMatrix[i]) {
                sprintf(name, "z%d%d", i, j);
                z[i][j] = IloNumVar(env, 0, 1, name);
                model.add(z[i][j]);
                model.add(IloConversion(env, x[i][j], ILOBOOL));
            }
        }
    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
        exit(EXIT_FAILURE);
    }

}

void Model::initModelHybrid() {
    cout << "Begin the model creation" << endl;
    cplex.setParam(IloCplex::Param::TimeLimit, 600);
    cplex.setParam(IloCplex::TreLim, 7000);
//    cplex.setOut(env.getNullStream());

    int root = 0;

    objectiveFunction();
    edgesLimitConstraintHybrid();
    outDegreeHybrid(root);
    branchesHybrid(root);
    branchesCorrelationHybrid(root);
    cout << "All done!" << endl;
}

void Model::edgesLimitConstraintHybrid() {
    IloExpr constraint(env);

    for (int i = 0; i < graph->n; i++)
        for (auto j : graph->incidenceMatrix[i])
            constraint += z[i][j];

    // Edges in a extreme vertex with degree one have to be one
    for (int i = 0; i < graph->n; i++)
        for (auto j : graph->incidenceMatrix[i])
            if (int(graph->incidenceMatrix[j].size()) == 1)
                model.add(z[i][j] == 1);

    model.add(constraint == (graph->n - 1));
}

void Model::outDegreeHybrid(int root){
    for (int v = 0; v < n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (int i = 0; i < n; i++){
                for (auto j : graph->incidenceMatrix[i]) {
                    if (j == v) constraint += z[i][v];
                }
            }
            model.add(constraint == 1);
        }
    }
}

void Model::branchesHybrid(int root){
    for (int v = 0; v < n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[v])
                constraint += z[v][u];

            model.add(constraint - 1 <= (int(incidenceMatrix[v].size()) - 2)*y[v]);
        } else {
            IloExpr constraint(env);
            for (auto u : incidenceMatrix[root])
                constraint += z[root][u];

            model.add(constraint - 2 <= (int(incidenceMatrix[root].size()) - 2)*y[v]);
        }
    }
}

void Model::branchesCorrelationHybrid(int root){
    for (int v = 0; v < n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[v])
                constraint += z[v][u];

            model.add(2*y[v] <= constraint);
        } else {
            IloExpr constraint(env);
            for (auto u : incidenceMatrix[root])
                constraint += z[root][u];

            model.add(2*y[root] <= constraint-1);
        }
    }
}

-------------------------------------------------------*/
