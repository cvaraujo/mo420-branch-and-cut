//
// Created by carlos on 16/11/2019.
//

#include "../inc/model.h"

Model::Model(Graph *graph) {
    if (graph != nullptr) {
        this->graph = graph;

//        initialize();
        initializeHybrid();

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
            sprintf(name, "y_%d", i);
            y[i] = IloNumVar(env, 0, 1, name);
            model.add(y[i]);
            model.add(IloConversion(env, y[i], ILOBOOL));
            for (auto j : graph->incidenceMatrix[i]) {
                if (i < j) {
                    sprintf(name, "x_%d_%d", i, j);
                    x[i][j] = x[j][i] = IloNumVar(env, 0, 1, name);
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
    for (int i = 0; i < graph->n; i++)
        for (auto j : graph->incidenceMatrix[i])
            if (int(graph->incidenceMatrix[j].size()) == 1)
                model.add(x[i][j] == 1);

}

void Model::setBranchConstraint() {
    for (int i = 0; i < graph->n; i++) {
        IloExpr constraint(env);
        for (auto j : graph->incidenceMatrix[i]) {
            constraint += x[i][j];
        }
        model.add((constraint - 2) <= (int(graph->incidenceMatrix[i].size()) - 2) * y[i]);
    }
    // Each vertex with degree less or equal to 2 cannot be a branche
    for (int i = 0; i < graph->n; i++)
        if (int(graph->incidenceMatrix[i].size()) <= 2) model.add(y[i] == 0);

    // Vertex with two or more bridges adjacent should be a branche and cut vertex with result in 3 or more CC
    for (int i = 0; i < graph->n; i++)
        if (graph->branches[i]) model.add(y[i] == 1);

    // Cocycle restriction
    for (auto p : graph->cocycle)
        model.add(x[p.first.u][p.first.v] + x[p.second.u][p.second.v] >= 1);

}

void Model::branchConstraintAdpt() {
    for (int i = 0; i < graph->n; i++) {
        IloExpr constraint(env);
        for (auto j : graph->incidenceMatrix[i]) {
            constraint += x[i][j];
        }
        model.add(2*y[i] <= constraint - 1);
    }
}


ILOLAZYCONSTRAINTCALLBACK2(Lazy, IloArray<IloNumVarArray>, x, Graph, graph) {
    try {
        IloEnv env = getEnv();

        vector <vector<IloNum >> val_x = vector<vector<IloNum>>(graph.n, vector<IloNum>(graph.n));

        for (int i = 0; i <graph.n; i++){
            for (auto j : graph.incidenceMatrix[i]){
                val_x[i][j] = getValue(x[i][j]);
            }
        }

        Graph g;
        g.n = graph.n;
        g.incidenceMatrix.resize(g.n);
        for (Edge e : graph.edges) {
            if (val_x[e.u][e.v] > EPS || val_x[e.v][e.u] > EPS) {
                g.incidenceMatrix[e.u].push_back(e.v);
                g.incidenceMatrix[e.v].push_back(e.u);
                g.edges.push_back(e);

            }
        }

        vector <vector<int>> comps;
        vector<int> nEdgesComponent;

        vector<int> color;
        color.resize(g.n, 0);

        for (int i = 0; i < g.n; i++) {
            if (color[i] != 0) continue;

            comps.push_back(vector<int>());
            nEdgesComponent.push_back(0);

            color[i] = 1;
            queue<int> q;
            comps.back().push_back(i);
            q.push(i);

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                for (int v : g.incidenceMatrix[u]) {
                    if (color[v] == 0) {
                        color[v] = 1;
                        q.push(v);
                        comps.back().push_back(v);
                    }
                    nEdgesComponent.back()++;
                }
            }

            /*Dividimos por 2, porque cada aresta foi contada 2 vezes*/
            nEdgesComponent.back() = nEdgesComponent.back() >> 1;
        }

        for (int i = 0; i < comps.size(); i++) {
            if (nEdgesComponent[i] >= comps[i].size()) {
                IloExpr cut(env);
                for (int u : comps[i]) {
                    for (int v : g.incidenceMatrix[u]) {
                        cut += (0.5 * x[u][v]);
                    }
                }
                add(cut <= (int(comps[i].size()) - 1));
            }
        }
    }
    catch (IloException &ex){
        cout << ex.getMessage() << endl;
    }
}

ILOUSERCUTCALLBACK5(Cut, IloArray<IloNumVarArray>, x, IloNumVarArray, y, Graph, graph, int, r18, int, r19){
    try {
        IloEnv env = getEnv();

		vector <vector<IloNum >> val_x = vector<vector<IloNum>>(graph.n, vector<IloNum>(graph.n));

	    for (int i = 0; i <graph.n; i++){
	        for (auto j : graph.incidenceMatrix[i]){
	            val_x[i][j] = getValue(x[i][j]);
	        }
	    }

	    IloNumArray val_y(env);
	    getValues(val_y, y);

	    /*Resticao (18) [2]*/
	    if (r18){
		    for (int i = 0; i < graph.n; i++){
		    	if (graph.incidenceMatrix[i].size() < 3) continue;
		    	vector<pair<float, int> > xe;
		    	for (int j : graph.incidenceMatrix[i]){
		    		xe.push_back(make_pair(val_x[i][j], j));
		    	}
		    	sort(xe.rbegin(), xe.rend());

		    	for (int k = 3; k < (int)graph.incidenceMatrix[i].size(); k++){
		    		float lhs = 2 - k;
		    		for (int j = 0; j < k; j++){
		    			lhs += xe[j].first;
		    		}

		    		if (lhs > 2 - EPS){
		                IloExpr cut(env);
		                cut += 2 - k;
			    		for (int j = 0; j < k; j++){
			    			cut += x[i][xe[j].second];
			    		}
			    		add(cut <= 2);
		    		}
		    	}
		    }
	    }

	    /*Restricao (19) [2]*/
	    if (r19){
		    for (int i : graph.cutVertices){
		    	int a = 0;
		    	if (a == i) a = 1;

		    	vector<int> color;
		 		color.resize(graph.n, 0);

		 		queue<int> q;
		 		q.push(a);
		 		color[a] = 1;

		 		while (!q.empty()){
		 			int u = q.front();
		 			q.pop();

		 			for (int v : graph.incidenceMatrix[u]){
		 				if (color[v] != 0 || v == i) continue;
		 				color[v] = 1;
		 				q.push(v);
		 			}
		 		}

		 		vector<int> A, B;
		 		for (int j = 0; j < graph.n; j++){
		 			if (j == i) continue;
		 			if (color[j] == 1) A.push_back(j);
		 			else B.push_back(j);
		 		}

		 		for (int j_ = 0; j_ < A.size(); j_++){
		 			for (int k_ = j_ + 1; k_ < A.size(); k_++){
		 				int j = A[j_], k = A[k_];
		 				if (val_x[i][j] + val_x[i][k] > 1 + val_y[i]){
							add(x[i][j] + x[i][k] <= 1 + y[i]);
		 				}
		 			}
		 		}

		 		for (int j_ = 0; j_ < B.size(); j_++){
		 			for (int k_ = j_ + 1; k_ < B.size(); k_++){
		 				int j = B[j_], k = B[k_];
		 				if (val_x[i][j] + val_x[i][k] > 1 + val_y[i]){
							add(x[i][j] + x[i][k] <= 1 + y[i]);
		 				}
		 			}
		 		}
		    }
	    }
    }
    catch (IloException &ex){
        cout << ex.getMessage() << endl;
    }
}
/*
ILOLAZYCONSTRAINTCALLBACK2(HLazy, IloArray<IloNumVarArray>, z, Graph, graph) {
    try {
        IloEnv env = getEnv();

        vector <vector<IloNum >> val_z = vector<vector<IloNum>>(graph.n, vector<IloNum>(graph.n));

        for (int i = 0; i <graph.n; i++){
            for (auto j : graph.incidenceMatrix[i]){
                val_z[i][j] = getValue(z[i][j]);
            }
        }

        Graph g;
        g.n = graph.n;
        g.incidenceMatrix.resize(g.n);
        for (Edge e : graph.edges) {
            if (val_x[e.u][e.v] > EPS) {
                g.incidenceMatrix[e.u].push_back(e.v);
            }
        }

        vector <vector<int>> comps;
        vector<int> nEdgesComponent;

        vector<int> color;
        color.resize(g.n, 0);

        for (int i = 0; i < g.n; i++) {
            if (color[i] != 0) continue;

            comps.push_back(vector<int>());
            nEdgesComponent.push_back(0);

            color[i] = 1;
            queue<int> q;
            comps.back().push_back(i);
            q.push(i);

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                for (int v : g.incidenceMatrix[u]) {
                    if (color[v] == 0) {
                        color[v] = 1;
                        q.push(v);
                        comps.back().push_back(v);
                    }
                    nEdgesComponent.back()++;
                }
            }

            /*Dividimos por 2, porque cada aresta foi contada 2 vezes
            nEdgesComponent.back() = nEdgesComponent.back() >> 1;
        }

        for (int i = 0; i < comps.size(); i++) {
            if (nEdgesComponent[i] >= comps[i].size()) {
                IloExpr cut(env);
                for (int u : comps[i]) {
                    for (int v : g.incidenceMatrix[u]) {
                        cut += (0.5 * x[u][v]);
                    }
                }
                add(cut <= (int(comps[i].size()) - 1));
            }
        }
    }
    catch (IloException &ex){
        cout << ex.getMessage() << endl;
    }
}
*/
void Model::solve(int sec, int r18, int r19) {
    /* Turn on traditional search for use with control callbacks */
//    cplex.setParam(IloCplex::Param::MIP::Strategy::Search, CPX_MIPSEARCH_TRADITIONAL);

    /* Desabilita paralelismo  */
//    cplex.setParam(IloCplex::Param::Threads, 1);


    cplex.setParam(IloCplex::Param::Preprocessing::Presolve, CPX_OFF);
    if (sec) this->cplex.use(Lazy(env, x, *graph));
    this->cplex.use(Cut(env, x, y, *graph, r18, r19));
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
            if (cplex.getValue(y[i]) >= 0.5)
                cout << "[" << i + 1 << ", " << cplex.getValue(y[i]) << "]" << endl;

    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
    }
}

/*-------------------------------------------------------*/

void Model::showSolutionHybrid() {
    try {
        cout << "Objective" << endl;
        cout << cplex.getObjValue() << endl;

        cout << "Selected edges" << endl;
        for (int i = 0; i < graph->n; i++) {
            for (auto j : graph->incidenceMatrix[i]) {
                if (cplex.getValue(z[i][j]) > 0.5) {
                    cout << "[" << i + 1 << ", " << j + 1 << "]" << endl;
                }
            }
        }

        cout << "Branch vertex" << endl;
        for (int i = 0; i < graph->n; i++)
            if (cplex.getValue(y[i]) >= 0.5)
                cout << "[" << i + 1 << ", " << cplex.getValue(y[i]) << "]" << endl;

    } catch (IloException &ex) {
        cout << ex.getMessage() << endl;
    }
}

void Model::initializeHybrid() {
    try {
        char name[20];
        this->model = IloModel(env);
        this->cplex = IloCplex(model);

        this->z = IloArray<IloNumVarArray>(env, graph->n);
        this->x = IloArray<IloNumVarArray>(env, graph->n);
        this->y = IloNumVarArray(env, graph->n);

        for (int i = 0; i < graph->n; i++) {
            this->z[i] = IloNumVarArray(env, graph->n);
            this->x[i] = IloNumVarArray(env, graph->n);
        }

        for (int i = 0; i < graph->n; i++) {
            sprintf(name, "y_%d", i);
            y[i] = IloNumVar(env, 0, 1, name);
            model.add(y[i]);
            model.add(IloConversion(env, y[i], ILOBOOL));
            for (auto j : graph->incidenceMatrix[i]) {
                if (i < j) {
                    sprintf(name, "x_%d_%d", i, j);
                    x[i][j] = x[j][i] = IloNumVar(env, 0, 1, name);
                    model.add(x[i][j]);
                    model.add(IloConversion(env, x[i][j], ILOBOOL));
                }
                sprintf(name, "z_%d_%d", i, j);
                z[i][j] = IloNumVar(env, 0, 1, name);
                model.add(z[i][j]);
                model.add(IloConversion(env, z[i][j], ILOBOOL));
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

    // (3)
    edgesLimitConstraint();
    // (4)
    setBranchConstraint();
    // (7)
    branchConstraintAdpt();
    // (27)-(29)
    outDegreeHybrid(root);
    branchesHybrid(root);
    branchesCorrelationHybrid(root);
    // (35)
    xAndZRelation();
    cout << "All done!" << endl;
}

void Model::outDegreeHybrid(int root) {
    for (int v = 0; v < graph->n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (auto j : graph->incidenceMatrix[v]) {
                constraint += z[j][v];
            }
            model.add(constraint == 1);
        }
    }
}

void Model::branchesHybrid(int root){
    for (int v = 0; v < graph->n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[v])
                constraint += z[v][u];
            model.add(constraint - 1 <= (int(graph->incidenceMatrix[v].size()) - 2)*y[v]);
        } else {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[root])
                constraint += z[root][u];
            model.add(constraint - 2 <= (int(graph->incidenceMatrix[root].size()) - 2)*y[v]);
        }
    }
}

void Model::branchesCorrelationHybrid(int root){
    for (int v = 0; v < graph->n; v++) {
        if (v != root) {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[v])
                constraint += z[v][u];

            model.add(2*y[v] <= constraint);
        } else {
            IloExpr constraint(env);
            for (auto u : graph->incidenceMatrix[root])
                constraint += z[root][u];

            model.add(2*y[root] <= constraint-1);
        }
    }
}

void Model::xAndZRelation() {
    for (int i = 0; i < graph->n; i++) {
        for (auto j : graph->incidenceMatrix[i]) {
            model.add(x[i][j] == z[i][j] + z[j][i]);
        }
    }
}

/*-------------------------------------------------------*/
