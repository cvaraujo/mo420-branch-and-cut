///* ******************************************************************
//   Arquivos  de exemplo  para  o desenvolvimento  de  um algoritmo  de
//   branch-and-cut   usando  o CPLEX.   Este  branch-and-cut resolve  o
//   problema  da mochila  0-1 e  usa  como cortes  as desigualdades  de
//   cobertura simples (cover inequalities) da  forma x(c) < |C| -1 onde
//   C é um conjunto de itens formando uma cobertura minimal.
//
//   Autor: Cid Carvalho de Souza
//          Rafael Ghussn Cano
//          Natanael Ramos
//          Instituto de Computação - UNICAMP - Brazil
//
//   Data: 11/2019
//
//   Arquivo: main.cpp
//   Descrição: arquivo com programas em C++
//   * *************************************************************** */
//
//
///* includes dos arquivos .h */
//
//#ifndef HGLOBAIS
//#include "globais.hpp"
//#endif
//
//#include "ilcplex/ilocplex.h"
//
//using namespace std;
//
///* variaveis globais */
//
///* dados da mochila */
//int n;              /* número de itens */
//vector<double> c;   /* custos */
//vector<int> w;      /* pesos */
//int W;              /* capacidade */
///* contador do total de cortes por nó da arvore B&B */
//int totcuts=0;
///* contador do total de lacos de separacao de cortes por nó da arvore B&B */
//int itersep=0;
///* profundidade maxima de um no da arvore de B&B em que sera feita separacao */
//int MAX_NODE_DEPTH_FOR_SEP=1000000;
///* - valor otimo da primeira relaxacao */
//double objval_relax;
///* - valor da relaxacao linear no final do 1o nó */
//double objval_node1;
///* Profundidade do nó */
//IloInt node_depth;
//
///* rotinas auxiliares */
//void errormsg(const char *sSubName,int nLineNo,int nErrCode);
//void ImprimeSol(const IloNumArray &val);
//void Mochila(const vector<double> &c, const vector<int> &w, int b, int n,
//             vector<int> &x, double *val);
//
///* Declaracao das callbacks */
//
///******************************************************************************\
// * InfoCallback para obter a profundidade do próximo nó a ser processado. Invocada
// * antes da seleção do próximo nó a ser explorado.
// * Autor: Natanael Ramos
// * Data: segundo semestre de 2019
//\******************************************************************************/
//ILONODECALLBACK0(Profundidade) {
//    /* Próximo nó a ser explorado sempre tem id 0. */
//    node_depth = getDepth(0);
//}
//
///******************************************************************************\
// * Rotina para separacao de Cover inequalities. Roda em todo nó.
// * Autor: Cid Carvalho de Souza
// * Data: segundo semestre de 2003
// * Adaptado para c++: Rafael Cano (03/2018)
// * Adaptação no método para obter profundidade do nó: Natanael Ramos (11/2019)
//\******************************************************************************/
//ILOUSERCUTCALLBACK1(Cortes, IloBoolVarArray, x) {
//    /* Recupera ambiente do cplex */
//    IloEnv env = getEnv();
//
//    /* Pega a solução do LP. */
//    IloNumArray val(env);
//    getValues(val, x);
//
//    /* verifica o número do nó em que se encontra */
//    NodeId node = getNodeId();
//
//    /* Imprime cabeçalho do nó */
//    printf("\n=========\n");
//    cout << "No " << node << "\n";
//    printf("\n=========\n");
//    printf("Laco de separacao: %d\n", itersep);
//
//    double lpobjval = getObjValue();
//
//    /* Imprime dados sobre o nó */
//    printf(".Valor otimo do LP: %12.6f\n", lpobjval);
//    printf(".Solucao otima do LP:\n");
//    ImprimeSol(val);
//    printf(".Rotina de separacao\n");
//
//    static bool first_node = true; /* Indica se é o primeiro nó */
//
//    /* guarda o valor da função objetivo no primeiro nó */
//    if (first_node && isAfterCutLoop()) {
//        first_node = false;
//        objval_node1 = lpobjval;
//    }
//
//    /* guarda o valor da função objetivo da primeira relaxação */
//    if (itersep==0) objval_relax = lpobjval;
//
//    /*  sai  fora se  a  profundidade do  nó  corrente  for maior  que
//     * MAX_NODE_DEPTH_FOR_SEP. */
//    printf(".Profundidade do nó: %d\n", node_depth);
//    if (node_depth > MAX_NODE_DEPTH_FOR_SEP)
//    {
//        printf(".Abortando separação por criterio de profundidade.");
//        return;
//    }
//
//    /* carga dos parametros para a rotina de separacao da Cover Ineq */
//
//    /* ATENCAO:  A rotina Mochila nao  usa a posicao  zero dos vetores
//       custo, peso  e sol,  portanto para carregar  o problema  e para
//       pegar a solucao eh preciso acertar os indices */
//
//    vector<int> peso(n+1, 0); /* +1 !! */
//    int capacidade = 0;
//
//    for(int i=0; i < n; i++){
//        peso[i+1] = w[i]; /* +1 !!! */
//        capacidade = capacidade + peso[i+1]; /* +1 !! */
//    }
//    capacidade = capacidade - 1 - W;
//
//    /* calcula custos para o problema de separação (mochila) */
//    double ajuste_val = 0.0; /* ajuste para o custo do pbm da separação */
//    vector<double> custo(n+1); /* +1 !!! */
//    for(int i=0; i<n; i++) {
//        custo[i+1] = 1.0 - val[i];
//        ajuste_val += custo[i+1];
//    }
//
//    vector<int> sol;  /* vetor solucao da rotina Mochila */
//    double moch_val;  /* valor da solucao da rotina Mochila */
//
//    /* resolve a mochila usando Programação Dinâmica */
//    Mochila(custo, peso, capacidade, n, sol, &moch_val);
//
//    /* calculo do RHS da desigualdade de cobertura */
//    int irhs=0;
//    for(int i=1; i<=n; i++)
//        if (sol[i]==0) irhs++;
//
//    /* verifica se a desigualdade estah violada */
//    if (ajuste_val - moch_val < 1.0-EPSILON) {
//        printf("..corte encontrado: (viol=%12.6f)\n\n   ", 1.0-ajuste_val+moch_val);
//        /* prepara a insercao do corte */
//        double drhs = (double)(irhs - 1.0);
//        int k = 0;
//        IloExpr cut(env);
//        for(int i=1; i<=n; i++)
//            if (!sol[i]) {
//                cut += x[i-1];
//                k++;
//                /* Impressão do corte */
//                printf("x[%d] ", i-1);
//                if (k == irhs) printf("<= %d\n\n", irhs-1);
//                else printf("+ ");
//            }
//        assert(k==irhs);
//
//        /* adiciona o corte */
//        add(cut <= drhs);
//        totcuts++;
//        itersep++;
//    } else
//        printf("..corte nao encontrado\n");
//
//    printf("..Fim da rotina de cortes\n");
//
//    /* salva um arquivo .lp com o LP atual */
//    // cpx_ret = CPXwriteprob(env, nodelp, "LPcuts.lp", "LP");
//    // if (cpx_ret)
//    //   errormsg("Main: Erro ao gravar arquivo do LP com cortes.\n", __LINE__, cpx_ret);
//}
//
///******************************************************************************\
// *  Rotina  que encontra  uma solução  heurística para  mochila binaria
// *  dada a solução e uma relaxação linear.
// *
// *  Autor: Cid Carvalho de Souza
// *  Data: 10/2003
// *  Adaptado para c++: Rafael Cano (03/2018)
// *
// *  Entrada: a solução fracionária corrente é $x$ e o nó corrente sendo
// *  explorado é o nó "node".
// *
// *  Idéia da heurística: ordenar itens em ordem decrescente dos valores
// *  de $x$. Em seguida ir construindo a solução heurística $xh$ fixando
// *  as variáveis em 1 ao varrer o vetor na ordem decrescente enquanto a
// *  capacidade da mochila permitir.
// *
//\******************************************************************************/
//
//struct RegAux {
//    double valor;
//    int indice;
//    friend bool operator<(const RegAux &a, const RegAux &b) {
//        return (a.valor > b.valor);
//    }
//};
//
//ILOHEURISTICCALLBACK1(HeuristicaPrimal, IloBoolVarArray, x) {
//    /* recupera solucao da relaxacao */
//    IloNumArray val(getEnv());
//    getValues(val, x);
//
//    /* vetor de registros */
//    vector<RegAux> xh(n);
//    for(int i=0;i<n;i++){
//        xh[i].valor=val[i];
//        xh[i].indice=i;
//    }
//
//    /* ordena em ordem decrescente de valores */
//    sort(xh.begin(), xh.end());
//
//    /* calcula custo e  compara com a melhor solucao  corrente. Se for
//     * melhor,  informa o  CPLEX sobre  o  novo incumbent  e salva  a
//     * solucao. */
//    double custo=0.0;
//    int Wresidual=W;
//    int i;
//    for(i=0;(i<n) && (Wresidual >= w[xh[i].indice]);i++){
//        Wresidual -= w[xh[i].indice];
//        custo += c[xh[i].indice];
//    }
//    /* Nota: ao final deste laço "i-1"  será o maior índice (em xh !) de
//       um item que cabe na mochila. */
//
//    /* Impressão da solução heurística encontrada */
//    printf("..Solucao Primal encontrada:\n");
//    for(int j=0;j<i;j++)
//        printf("item %3d, peso %6d, custo %12.6f\n",
//               xh[j].indice,w[xh[j].indice],c[xh[j].indice]);
//    printf("               %6d        %12.6f\n",
//           W-Wresidual,custo);
//
//
//    /* informa a solução CPLEX */
//    for(int j=0;j<i;j++)
//        val[xh[j].indice]=1.0;
//
//    for(int j=i;j<n;j++)
//        val[xh[j].indice]=0.0;
//
//    setSolution(x, val, custo);
//}
//
///******************************************************************************\
// * Exemplo para LazyConstraints
// ******************************************************************************/
//ILOLAZYCONSTRAINTCALLBACK0(LazyConstraints) {
//    printf("\n *** Lazy Constraint\n");
//}
//
///* mensagem de uso do programa */
//void showUsage() {
//    printf ("Uso: knap <estrategia> [prof_max_para_corte] < instancia \n");
//    printf ("- estrategia: string de \"0\"\'s e \"1\"\'s de tamanho 2.\n");
//    printf ("  - 1a. posicao = \"1\" se a heuristica primal deve ser usada. \n");
//    printf ("  - 2a. posicao = \"1\" se a separacao das \"cover inequalities\" deve ser usada.\n");
//    printf ("    Nota: estrategia=\"00\" equivale a um Branch-and-Bound puro\n");
//    printf ("- prof_max_para_corte: maior altura de um no para aplicar cortes\n");
//    printf ("  (se omitido assume o valor default = 1000000) \n");
//    printf ("- instancia: nome do arquivo contendo a instancia \n");
//}
//
///* ============================ */
///* Inicio do programa principal */
///* ============================ */
//int main(int argc, char * argv[]) {
//
//    /* variaveis auxiliares */
//    char opcoes[2]="";
//    char probname[] = "mochila";
//    /* - diz se usara ou nao a heuristica primal */
//    bool HEURISTICA_PRIMAL;
//    /* - diz se usara ou nao cortes */
//    bool BRANCH_AND_CUT;
//
//
//    /* ambiente do cplex */
//    IloEnv env;
//
//    /* Verifica estratégia a adotar de acordo com a linha de comando */
//
//    if ( (argc<2) || (argc>3) ) {
//        showUsage();
//        exit(-1);
//    }
//
//    if ( (strlen(argv[1]) != 2) ){
//        printf("Primeiro parametro tem tamanho menor que 2. \n");
//        showUsage();
//        exit(-1);
//    }
//    else{
//        sprintf(opcoes,"%c",argv[1][0]);
//        if ( (strcmp(opcoes,"0")) && (strcmp(opcoes,"1")) ){
//            printf("Primeiro parametro dever ter apenas \"0\"s e \"1\"s !\n");
//            showUsage();
//            exit(-1);
//        }
//        else HEURISTICA_PRIMAL=(strcmp(opcoes,"0") != 0);
//
//        sprintf(opcoes,"%c",argv[1][1]);
//        if ( (strcmp(opcoes,"0")) && (strcmp(opcoes,"1")) ){
//            printf("Primeiro parametro dever ter apenas \"0\"s e \"1\"s !\n");
//            showUsage();
//            exit(-1);
//        }
//        else BRANCH_AND_CUT=(strcmp(opcoes,"0") != 0);
//    }
//
//    if (argc>=3) MAX_NODE_DEPTH_FOR_SEP=atoi(argv[2]);
//
//    /* inicializa valores de variaveis globais */
//    totcuts=0;   itersep=0;
//
//    /* le dados de entrada do problema da Mochila 0-1.*/
//    scanf("%d %d",&n,&W);
//    c.resize(n);
//    w.resize(n);
//    for(int i=0;i<n;i++) scanf("%lf",&c[i]);
//    for(int i=0;i<n;i++) scanf("%d",&w[i]);
//
//    /* objeto que representa o modelo */
//    IloModel model(env);
//
//    /* cria variaveis do modelo */
//    IloBoolVarArray x(env, n);
//
//    /* funcao objetivo */
//    IloExpr obj(env);
//    for(int i=0;i<n;i++)
//        obj += c[i]*x[i];
//    model.add(IloMaximize(env, obj));
//
//    /* restricao (soh tem uma!) */
//    IloExpr constr(env);
//    for (int i=0;i<n;i++)
//        constr += w[i]*x[i];
//    model.add(constr <= W);
//
//    /* cria objeto do cplex */
//    IloCplex cplex(env);
//
//    /* carrega o modelo */
//    cplex.extract(model);
//
//    /* desabilita output */
//    // cplex.setOut(env.getNullStream());
//
//    /* ======================================================================== */
//    /* Atribui valores a vários parametros de controle do CPLEX                 */
//    /* ======================================================================== */
//
//    /* limita o tempo de execucao */
//    cplex.setParam(IloCplex::Param::TimeLimit, MAX_CPU_TIME);
//
//    /* Desabilita o PRESOLVE: o problema da mochila é muito fácil para o CPLEX */
//    cplex.setParam(IloCplex::Param::Preprocessing::Presolve, CPX_OFF);
//
//    /* Desabilita heurísticas: o problema da mochila é muito fácil para o CPLEX */
//    cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, -1);
//    cplex.setParam(IloCplex::Param::MIP::Strategy::RINSHeur, -1);
//    cplex.setParam(IloCplex::Param::MIP::Strategy::FPHeur, -1);
//
//    /* Assure linear mappings between the presolved and original models */
//    /* Colocado em 0 quando lazy constraints são utilizadas (podem ser acionadas
//     * durante o presolve) */
//    cplex.setParam(IloCplex::Param::Preprocessing::Linear, 0);
//
//    /* Turn on traditional search for use with control callbacks */
//    cplex.setParam(IloCplex::Param::MIP::Strategy::Search, CPX_MIPSEARCH_TRADITIONAL);
//
//    /* Desabilita paralelismo  */
//    cplex.setParam(IloCplex::Param::Threads, 1);
//
//    /* salva um arquivo ".lp" com o LP original */
//    // cplex.exportModel("LP.lp");
//
//    /* impressão para conferência */
//    if (HEURISTICA_PRIMAL) {
//        printf("*** Heuristica Primal sera usada\n");
//        cplex.use(HeuristicaPrimal(env, x));
//    }
//
//    if (BRANCH_AND_CUT) {
//
//        printf("*** Algoritmo de branch-and-cut.\n");
//
//        /* callback para obter a profundidade do nó atual */
//        cplex.use(Profundidade(env));
//        /* o prolema da mochila nao inclui lazy constraints mas fica o exemplo
//           callback indicando que sera feita separacao de lazy constraints em cada
//           nó da arvore de B&B */
//        cplex.use(LazyConstraints(env));
//
//        /* callback indicando que sera feita separacao de cortes em cada
//           nó da arvore de B&B */
//        cplex.use(Cortes(env, x));
//
//    } else {
//        printf("*** Algoritmo de branch-and-bound puro.\n");
//    }
//
//    /* Desabilita cortes do CPLEX. Mochila é muito fácil para o CPLEX */
//    cplex.setParam(IloCplex::Param::MIP::Limits::CutsFactor, 1.0);
//
//    /* resolve  o  problema: retorna  true  se  uma solucao  viavel  foi
//       encontrada (nao necessariamente otima), false caso contrario */
//    double zstar = 0.0;
//    IloNumArray xstar(env);
//    int incumbent_node = -1;
//    if (cplex.solve()) {
//        /* recupera melhor solucao */
//        cplex.getValues(xstar, x);
//        ImprimeSol(xstar);
//        zstar = cplex.getObjValue();
//        incumbent_node = cplex.getIncumbentNode();
//
//        printf("\n");
//        printf("- Valor da solucao otima =%12.6f \n", zstar);
//        printf("- Variaveis otimas: (no=%d)\n", incumbent_node);
//
//    } else  printf("Main: programa terminou sem achar solucao inteira !\n");
//
//    /* impressao de estatisticas */
//    printf("********************\n");
//    printf("Estatisticas finais:\n");
//    printf("********************\n");
//    printf(".total de cortes inseridos ........ = %d\n", totcuts);
//    printf(".valor da FO da primeira relaxacao. = %.6f\n", objval_relax);
//    printf(".valor da FO no no raiz ........... = %.6f\n", objval_node1);
//
//    printf(".total de nos explorados .......... = %d\n", int(cplex.getNnodes()));
//    printf(".no da melhor solucao inteira ..... = %d\n", incumbent_node);
//    printf(".valor da melhor solucao inteira .. = %d\n", (int)(zstar+0.5));
//    /* somar 0.5 evita erros de arredondamento */
//
//    /* verifica o valor do melhor_limitante_dual */
//    double melhor_limitante_dual = cplex.getBestObjValue();
//
//    if (melhor_limitante_dual < zstar + EPSILON)
//        melhor_limitante_dual = zstar;
//    printf(".melhor limitante dual ............ = %.6f\n", melhor_limitante_dual);
//
//    printf("========================================\n");
//
//    return 0;
//}
//
///******************************************************************************\
// *  Rotina que resolve uma mochila binaria por Programação Dinâmica.
// *  Autor: Cid Carvalho de Souza
// *  Data: 10/2002
// *  Adaptado para c++ por: Rafael Cano
// *  Data: 03/2018
// *
// *  Objetivo: resolver uma mochila binaria maximizando o custo cx
// *  e com uma restricao do tipo wx <= b.
// *
// *  Entrada: vetores $c$ (custos), $w$ (pesos), $b$ (capacidade)
// *  e $n$ numero de itens.
// *
// *  Saidas: vetor $x$ (solucoes) e $z$ (valor otimo).
// *
// *  Observacao: todos os vetores sao inteiros, exceto os custos
// *  que sao "double", assim como o valor otimo $z$.
// *
// *  IMPORTANTE: todos os  vetores comecam na posicao zero  mas os itens
// *  são supostamente numerados de 1 a n. Assim, c[1] é o custo do item 1.
// *
//\******************************************************************************/
//
//void Mochila(const vector<double> &c, const vector<int> &w, int b, int n,
//             vector<int> &x, double *val) {
//
//    /* cria matriz de resultados intermediarios da Prog. Din */
//    vector< vector<double> > z(n+1, vector<double>(b+1, 0.0));
//
//    /* calculo do "limite" (numero negativo cujo valor absoluto eh
//       maior que o valor otimo com certeza): foi inicializado com ZERO. */
//    double limite = 0.0;
//    for(int k=1;k<=n;k++) limite -= c[k];
//
//    /* prog. din.: completando a matriz z */
//    for(int k=1; k<=n; k++)
//        for(int d=1; d<=b; d++){
//            /* aux sera igual ao valor de z[k-1][d-w[k]] quando esta celula
//               existir ou sera igual ao "limite" caso contrario. */
//            double aux = (d-w[k]) < 0 ? limite : z[k-1][d-w[k]];
//            z[k][d] = (z[k-1][d] > aux+c[k]) ? z[k-1][d] : aux+c[k];
//        }
//
//    /* carrega o vetor solucao */
//    x.assign(n+1, 0);
//
//    int d=b, k=n;
//    while ((d!=0) && (k!=0)) {
//        if (z[k-1][d] != z[k][d]) {
//            d=d-w[k];   x[k]=1;
//        }
//        k--;
//    }
//    *val=z[n][b];
//}
//
///******************************************************************************\
// *  Rotina que imprime uma solução heurística para mochila binaria
// *  dada a solução e uma relaxação linear.
// *
// *  Autor: Cid Carvalho de Souza
// *  Data: 10/2003
///******************************************************************************/
//void ImprimeSol(const IloNumArray &val){
//    for(int i=0;i<n;i++) if (val[i] > EPSILON)
//            printf("x[%3d]=%12.6f (w[%3d]=%6d, c[%3d]=%12.6f)\n",i,val[i],i,w[i],i,c[i]);
//}

#include "inc/graph.h"
#include "inc/model.h"

int main(int argc, char *argv[]) {
    Graph *graph = new Graph();
    graph->load_graph("toy.txt");
//    graph->twoCocycle();
    graph->print_graph();
    Model *model = new Model(graph);
    model->initModel();
    model->solve();
    model->showSolution();
    return 0;
}