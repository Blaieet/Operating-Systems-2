//Imports necesaris per la sortida, per la manipulacio d'strings, pel l'identificacio de caracters i per l'arbre
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "red-black-tree.h"
#include <ctype.h> 

#define MAXLINE 100 //Mida maxima

/*
	Funcio que insereix una paraula en l'arbre sempre i quant no estigui present ja en ell. 
	Si ja esta afegida, incrementa la variable que mante el numero de vegades que esta repetida
	Rep per parametre la paraula i l'arbre, i no retorna res
	Es comprova si la paraula esta afegida amb el metode findNode. Si aquest es nul, la paraula no existeix.
*/
void afegirParaula(char *paraula,RBTree *tree){
	RBData *treeData;
	//Reserva de memoria "a mida" per la paraula
	int len = strlen(paraula);
	char *a = (char *)malloc(sizeof(char)*len);
	strcpy(a,paraula);
	//busquem si existeix el node
	treeData = findNode(tree,paraula);

	if(treeData !=NULL){
		treeData->num_vegades++;
	}else{
		treeData = malloc(sizeof(RBData)); //"Creem el node"
		treeData->key = a;
		treeData->num_vegades = 1;
		insertNode(tree,treeData); //S'insereix
	}		
}

/*
	Main de l'apliacio
	Per ordre, es declaren les variables necesaries, s'obre la canonada, s'inicialitza l'arbre, 
	s'extraixen i s'insereixen les paraules i finalment s'allibera memoria
*/
int main(int argc,char **argv){
	
	char line[MAXLINE];
	//Reserva de memoria per la paraula 
	char *paraula = malloc(sizeof(char)*MAXLINE);

	//Amb largument(el pdf) pasat per consola creem la comanda a executar a popen
	char *comanda = malloc(sizeof(char)*20);
	char *comanda2 = malloc(sizeof(char)*20);
	strcpy(comanda,"pdftotext ");
	comanda2 = argv[1];
	strcat(comanda,comanda2);
	strcat(comanda," -"); //Perque la sortida de pdftotext es faci per la sortida estandard

	FILE *fpout; //Per la canonada
	int len,i,par,letra_rara; //Len mantindra la longitud. i la longitud de la paraula, par sera un contador extern i lletra_rara "un boolea" per si la paraula no es valida
	letra_rara = 1;
	fpout = popen(comanda, "r"); 

	//Declaracio i inicialitzacio de l'arbre
	RBTree *tree;

  	/* Allocate memory for tree */
  	tree = (RBTree *) malloc(sizeof(RBTree));

  	/* Initialize the tree */
  	initTree(tree);

	if (!fpout)
	{
		printf("ERROR: no puc crear canonada.\n");
		exit(EXIT_FAILURE);
	}
	
	while (fgets(line, MAXLINE, fpout) != NULL) {
		
		len = strlen(line);
		par = 0;
  		for(i = 0; i < len; i++){
  			//Si el caracter es una lletra
			if ((isalpha(line[i]))){
                		if(isupper(line[i])){
                  			line[i] = tolower(line[i]); //Posala en minuscula en cas de que estigui en majuscules
                		}
				if(letra_rara){ //Si em detectat que es una lletra rara, ves a la paraula seguent
					paraula[par] = line[i];
					par++;
				}
			}
			else if (isdigit(line[i])){ //Si el caracter es un numero, ignora la paraula
				par = 0; 
				memset(paraula,0,MAXLINE);

			} else if (ispunct(line[i])){ //Si el caracter es un signe de puntuacio, afageix la paraula (sense el signe)
				if(strlen(paraula)>0){
					afegirParaula(paraula,tree);
				}
				par = 0;
				letra_rara = 1;
				memset(paraula,0,MAXLINE);
			}else if(isspace(line[i])){ //Si el caracter es un espai, insereix en l'arbre la paraula que te derrera
				par = 0;
				letra_rara = 1;
				//printf("%s\n",paraula);
				if(strlen(paraula)>0){
					afegirParaula(paraula,tree);
				}
				memset(paraula,0,MAXLINE);
			}else{ //Si no coneixem aquest caracter, ignorem la paraula
				par = 0;
				memset(paraula,0,MAXLINE);
				strcpy(paraula,"");
				letra_rara = 0;

			}
		}

	}

	free(paraula);

	if (pclose(fpout) == -1){
		printf("ERROR: pclose.\n");
		exit(EXIT_FAILURE);
	}
	
	//Funcio per comprovar el funcionament del codi. Busca per la paraula "text" i imprimeix el numero de vegades que esta repetida en el pdf

	/*
	RBData *treeData;
	treeData = findNode(tree,"text");
	if (treeData != NULL) {
		int f = (int) treeData->num_vegades;
		printf("%d\n",f);
    	} else {
		printf("Element no trobat\n");
	}
	*/

	/* Delete the tree */
  	deleteTree(tree);
  	free(tree);
	
	printf("L'aplicació ha acabat.\n");

	return 0;
}

































