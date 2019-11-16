/* ******************************************************************
   Arquivos  de exemplo  para  o desenvolvimento  de  um algoritmo  de
   branch-and-cut   usando  o CPLEX.   Este  branch-and-cut resolve  o
   problema  da mochila  0-1 e  usa  como cortes  as desigualdades  de
   cobertura simples (cover inequalities) da  forma x(c) < |C| -1 onde
   C é um conjunto de itens formando uma cobertura minimal.

   Autor: Cid Carvalho de Souza
          Rafael Ghussn Cano
          Instituto de Computação - UNICAMP - Brazil

   Data: primeiro semestre de 2018

   Arquivo: global.hpp
   Descrição: declarações globais.  
 * *************************************************************** */

/* includes e estruturas globais */
#define HGLOBAIS 1

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

/* Include para  o uso  da biblioteca  assert.  Para  compilar  com os
   asserts, comentar a linha abaixo. */
/*#define NDEBUG 1*/
#include <cassert>

/* Constantes para definir estratégias de corte */
#define EPSILON 0.000001 /* uma tolerância usada em alguns cálculos com doubles*/
#define MAX_CPU_TIME 1800 /* limitando o tempo de CPU */

