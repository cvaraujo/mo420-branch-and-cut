#include "inc/graph.h"
#include "inc/model.h"

int main(int argc, char *argv[]) {
	if (argc != 3){
		printf(".\\MO420_Branch_and_Cut abcde <entrada>\n");
		printf("a = hybrid\n");
		printf("b = 18 [2]\n");
		printf("c = 19 [2]\n");
		printf("d = 34 [2]\n");
		printf("e = heuristic\n");
		exit(1);
	}
    Graph *graph = new Graph();
    int hybrid = argv[1][0] - '0';
    int r18 = argv[1][1] - '0';
    int r19 = argv[1][2] - '0';
    int r34 = argv[1][3] - '0';
    int heuristic = argv[1][4] - '0';
    graph->load_graph(argv[2]);
    graph->twoCocycle();
//    graph->print_graph();
    Model *model = new Model(graph);
    if (hybrid) model->initModelHybrid();
    else model->initModel();
    model->solve(r18, r19, r34, heuristic);
    if (hybrid) model->showSolutionHybrid();
    else model->showSolution();

    string path = "results/";
    path += argv[1];
    path += "/";
    path += argv[2];
    ofstream myfile;
    myfile.open(path);

    for (int i = 1; i < argc; i++){
    	myfile << argv[i] << " ";
    }

    myfile << endl << model->qtd_sec << endl;

    myfile << model->qtd_18 << endl;
    myfile << model->qtd_19 << endl;
    myfile << model->qtd_34 << endl;

    myfile << model->relax1 << endl;
    myfile << model->obj1 << endl;

    myfile << model->cplex.getNnodes() << endl;
    myfile << "missing" << endl;

    myfile << model->cplex.getObjValue() << endl;
    myfile << model->cplex.getBestObjValue() << endl;

    myfile << model->cplex.getTime() << endl;

    /*Tirar depois*/

    myfile << endl;

    int cont = 0;
    for (int i = 0; i < model->graph->n; i++){
    	cont += model->graph->branches[i];
    }

    myfile << cont << endl;

    int cont_b = model->graph->bridges.size(), cont_c = 0;
    for (int i = 0; i < model->graph->n; i++){
    	for (int j : model->graph->incidenceMatrix[i]){
    		if (i < j && model->graph->isBridgeAndCocycle[i][j]) cont_c++;
    	}
    }

    myfile << cont_b << endl;
    myfile << cont_c - cont_b << endl;

    myfile.close();

    delete graph;
    return 0;
}