/**
 *
 * Practica 3 
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "red-black-tree.h"
#include <ctype.h> 

#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9

//Variable global de l'arbre que fem servir en tot moment
RBTree *tree;

//Funcio que crea, destrueix o crea-i-destrueix l'arbre. D'aquesta manera no ho fem dins d'altres funcions!
//administrar == 1: destrueix l'arbre (opcio sortir)
//administrar == 0 i hi ha arbre: destrueixlo i tornar a crear. Si no, creal de zero
void administrarTree(int administrar){
	if(administrar){
		deleteTree(tree);
		free(tree);
	}else if (tree) {
		deleteTree(tree);
		free(tree);
		/* Allocate memory for tree */
		tree = (RBTree *) malloc(sizeof(RBTree));
		/* Initialize the tree */
		initTree(tree);
	}else{
	  	/* Allocate memory for tree */
		tree = (RBTree *) malloc(sizeof(RBTree));
		/* Initialize the tree */
		initTree(tree);
	}
}
/**
 * 
 *  Menu
 * 
 */

int menu() 
{
    char str[5];
    int opcio;
	
	printf("\n\n*****************************************\n");
    printf("Menu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio d'un node\n");
    printf(" 5 - Sortir\n\n");
	printf("*****************************************\n");
    printf("   Escull opcio: ");

    fgets(str, 5, stdin);
    opcio = atoi(str); 

    return opcio;
}

//Funcio que afageix una paraula (node) a l'arbre, juntament amb el numero de vegades que esta repetida
void afegirParaula(char *paraula){
	RBData *treeData;
	int len = strlen(paraula);
	char *a = (char *)malloc(sizeof(char)*len);
	strcpy(a,paraula);
	//busquem si existeix el node
	treeData = findNode(tree,paraula);

	if(treeData !=NULL){
		treeData->num_vegades++;
	}else{
		treeData = malloc(sizeof(RBData));
		treeData->key = a;
		treeData->num_vegades = 1;
		insertNode(tree,treeData);
	}		
}

//Funcio que llegeix un fitxer PDF amb pdftotext i insereix crida a la funcio per inserir paraules si aquesta respecte les normes
//Es la mateixa que la practica pasada
void llegirPdf(char str[]){

	char line[MAXLINE];
	char *paraula = malloc(sizeof(char)*MAXLINE);
	char *comanda = malloc(sizeof(char)*20);
	char *comanda2 = malloc(sizeof(char)*20);
	comanda2 = str;
	strcpy(comanda,"pdftotext ");
	strcat(comanda,comanda2);
	strcat(comanda," -");

	FILE *fpout;
	int len,i,par,letra_rara;
	letra_rara = 1;
	fpout = popen(comanda, "r");

	if (!fpout)
	{
		printf("ERROR: no puc crear canonada.\n");
		exit(EXIT_FAILURE);
	}
	while (fgets(line, MAXLINE, fpout) != NULL) {
		len = strlen(line);
		par = 0;
  		for(i = 0; i < len; i++){
			if ((isalpha(line[i]))){
				if(isupper(line[i])){
					line[i] = tolower(line[i]);
                }
			if(letra_rara){
					paraula[par] = line[i];
					par++;
				}
			}
			else if (isdigit(line[i])){
				par = 0;
				memset(paraula,0,MAXLINE);

			}else if (ispunct(line[i])){
				if(strlen(paraula)>0){
					afegirParaula(paraula);
				}
				par = 0;
				letra_rara = 1;
				memset(paraula,0,MAXLINE);
			}else if(isspace(line[i])){
				par = 0;
				letra_rara = 1;
				if(strlen(paraula)>0){
					afegirParaula(paraula);
				}
				memset(paraula,0,MAXLINE);
			}else{
				par = 0;
				memset(paraula,0,MAXLINE);
				strcpy(paraula,"");
				letra_rara = 0;

			}
		}
	}
	//Comentats perque sino Valgrind no funciona (vam enviar correu pero no vam aconseguir res)

	//free(comanda2);
	//free(comanda);
	//free(paraula);


	if (pclose(fpout) == -1){
		printf("ERROR: pclose.\n");
		exit(EXIT_FAILURE);
	}
}

//Funcio que llegeix un fitxer amb noms d'altres fitxers pdf's els quals va pasant per la funcio llegirPdf per agafar les seves paraules
void llegirTxt(char str[]){
	administrarTree(0); //Creem l'arbre

	FILE *fp;
	int i, first_number;
	char str2[MAXLINE];

	/* opening file for reading */
	fp = fopen(str , "r");
	if(fp == NULL) {
		perror("Error opening file");
	}else{
		fgets(str, 100, fp);
		first_number = atoi(str); //Llegim quans fitxers hi ha

		for(i = 0; i < first_number; i++) {
			fgets(str2, 100, fp); //Agafem el nom del fitxer pdf
			str2[strlen(str2)-1] = 0;
			llegirPdf(str2);
		}
		fclose(fp);
	}
}


//Funcio recursiva que va guardant la informacio de cada node a un fitxer
void guardarRecursiu(Node *a,FILE *fp) {

    int len;
	len = strlen(a->data->key);

	//Guardant longitud paraula
	fwrite(&len, sizeof(int), 1, fp);
	//Guardant paraula
	fwrite(a->data->key, sizeof(char),len,fp);
	//Guardant cops que surt
	int numcops = a->data->num_vegades;
	fwrite(&numcops, sizeof(int), 1,fp);

	if(a->right != NIL) {
		guardarRecursiu(a->right,fp);
	}
	if (a->left != NIL) {
		guardarRecursiu(a->left,fp);
	}

}

//Funcio que pasat un fitxer hi guarda el magic number i el numero de nodes de l'arbre. Un cop fet, crida la funcio recursiva
//Per guardar la informacio de cada node
void guardarArbre(char *filename) {
	
	FILE *fp;
	fp = fopen(filename, "w");
	
	if(!fp) {
		printf("Fitxer erroni!\n");
	}else{
		//Guardat nombre magic
		int magic = (int)MAGIC_NUMBER;
		fwrite(&magic,sizeof(int),1,fp);
	
		//Guardat num nodes
		int numNodes;
		numNodes = tree->numNodes;
	
		fwrite(&numNodes, sizeof(int), 1, fp);
	    
		guardarRecursiu(tree->root,fp);
	    
		fclose(fp);
	}
}

//Funcio que carrega un arbre que es troba guardat en un fitxer. 
void carregaArbre(char *filename) {
	administrarTree(0); //Mira si s'ha de destruir i tornar a crear l'arbre
	int i,longparaula,repeticionsparaula;
	FILE *fp;
	fp = fopen(filename, "r");
	if (!fp) {
		printf("Error amb el fitxer de lectura\n");
	}else{
		//Llegint nombre magic
		int magicLlegit;
		fread(&magicLlegit, 1, sizeof(int),fp);
		//Comprovacio de nombre magic
	
		if(magicLlegit == MAGIC_NUMBER){
			//Llegint numNodes
			int numNodes;
			fread(&numNodes, 1, sizeof(int),fp);

			RBData *treeData;
			for(i=0;i < numNodes;i++){
		
				//Node
				treeData = malloc(sizeof(RBData));
		
				//Len Paraula
				fread(&longparaula,sizeof(int),1,fp);
		
				//Paraula
				treeData->key = malloc(sizeof(char)*(longparaula+1));
				fread(treeData->key,sizeof(char),longparaula,fp);
				treeData->key[longparaula] = 0;

				//Repeticions
				fread(&repeticionsparaula,sizeof(int),1,fp);
				treeData->num_vegades = repeticionsparaula;
				//comprovem que no es llegeixin nombres negatius o zero
				if(longparaula>0 && repeticionsparaula > 0){
					insertNode(tree,treeData);
				}
		
			}
		}

		fclose(fp);
	}

}

//Funcio que donada una paraula la busca a l'arbre. Retorna el numero de vegades que aquesta es troba repetida en el text sempre i quant la trobi
void findParaula(char str[]){
	if(tree !=NULL){
		RBData *treeData;
		treeData = findNode(tree,str);
		if (treeData != NULL) {
			int f = (int) treeData->num_vegades;
			printf("La paraula buscada es repeteix: %d vegades \n",f);
	    } else {
			printf("element no trobat\n");
		}
	}else{
		printf("La paraula no existeix.\n");
	}
}
/**
 * 
 *  Main procedure
 *
 */

int main(int argc, char **argv)
{
    char str[MAXLINE];
    int opcio;

    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");

    do {
        opcio = menu();
        printf("\n\n");

        /* Feu servir aquest codi com a pantilla */

        switch (opcio) {
            case 1: //Llegir pdf's
                printf("Introdueix fitxer que conte llistat fitxers PDF: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;
				llegirTxt(str);

                break;

            case 2: //Guardar l'arbre a memoria
				if(!tree) {
					printf("Ep, que no hi ha cap arbre creat! \n");
					break;
				}

                printf("Introdueix el nom de fitxer en el qual es desara l'arbre: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;
				guardarArbre(str);
                break;

            case 3: //Recuperar l'arbre des d'un fitxer

				printf("Introdueix nom del fitxer amb l'arbre: ");
				fgets(str, MAXLINE, stdin);
				str[strlen(str)-1]=0;
				printf("Carregant... Espera\n");
				carregaArbre(str);

                break;

            case 4: //Cerca d'informacio de l'arbre
                printf("Introdueix la paraula: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;
				findParaula(str);

                break;

            case 5: //Sortir

                administrarTree(1);

                break;

            default:
                printf("Opcio no valida\n");

        } /* switch */
    }
    while (opcio != 5);

    return 0;
}

