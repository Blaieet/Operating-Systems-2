/**
 *
 * Practica 3 
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#include "red-black-tree.h"

#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9
#define NUMERO_FILS 2

pthread_t ntid[NUMERO_FILS];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
int count = 0;

struct parametres{
    int fil;
    int num_elements;
    char **llistat_pdf;
    RBTree *arbreGran;
    RBTree *arbreLocal;
};

/**
 *
 *  Counts the number of nodes of a tree
 *
 */

int count_nodes_recursive(Node *x)
{
    int nodes;

    nodes = 0;

    /* Analyze the children */
    if (x->right != NIL)
        nodes += count_nodes_recursive(x->right);

    if (x->left != NIL)
        nodes += count_nodes_recursive(x->left);

    /* Take into account the node itself */
    nodes += 1;

    return nodes;
}

int count_nodes(RBTree *tree)
{
    int nodes;

    nodes = count_nodes_recursive(tree->root);

    return nodes;
} 


/**
 *
 *  Process each line that is received from pdftotext: extract the
 *  words that are contained in it and insert them in the tree.
 *
 */

void process_line(char *line, RBTree *tree)
{
    RBData *tree_data;

    int i, j, is_word, len_word, len_line;
    char paraula[MAXLINE], *paraula_copy;

    i = 0;

    len_line = strlen(line);

    /* Search for the beginning of a candidate word */

    while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i])))) i++; 

    /* This is the main loop that extracts all the words */

    while (i < len_line)
    {
        j = 0;
        is_word = 1;

        /* Extract the candidate word including digits if they are present */

        do {

            if (isalpha(line[i]))
                paraula[j] = line[i];
            else 
                is_word = 0;

            j++; i++;

            /* Check if we arrive to an end of word: space or punctuation character */

        } while ((i < len_line) && (!isspace(line[i])) && (!ispunct(line[i])));

        /* If word insert in list */

        if (is_word) {

            /* Put a '\0' (end-of-word) at the end of the string*/
            paraula[j] = 0;

            /* Now transform to lowercase */
            len_word = j;

            for(j = 0; j < len_word; j++)
                paraula[j] = tolower(paraula[j]);

            /* Search the work in the tree */
            tree_data = findNode(tree, paraula);

            if (tree_data != NULL) {
                //printf("Incremento comptador %s a l'arbre\n", paraula);

                /* We increment the number of times current item has appeared */
                tree_data->num_vegades++;

            } else {
                //printf("Insereixo %s a l'arbre\n", paraula);

                /* If the key is not in the list, allocate memory for the data and
                 * insert it in the list */

                paraula_copy = malloc(sizeof(char) * (len_word+1));
                strcpy(paraula_copy, paraula);

                tree_data = malloc(sizeof(RBData));
                tree_data->key = paraula_copy;
                tree_data->num_vegades = 1;

                insertNode(tree, tree_data);
            }
        } /* if is_word */

        /* Search for the beginning of a candidate word */

        while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i])))) i++; 

    } /* while (i < len_line) */
}

RBTree *create_tree(char *pdf){

    FILE *fp_pipe;
    RBTree *tree;

    int i, num_pdfs;
    char line[MAXLINE], command[MAXLINE];

    /* Allocate memory for tree */
    tree = (RBTree *) malloc(sizeof(RBTree));

    /* Initialize the tree */
    initTree(tree);

    printf("Processant fitxer %s\n", pdf);

    /** This is the command we have to execute. Observe that we have to specify
     * the parameter "-" in order to indicate that we want to output result to
     * stdout.  In addition, observe that we need to specify \n at the end of the
     * command to execute. 
     */

    sprintf(command, "pdftotext %s -\n", pdf);
    fp_pipe = popen(command, "r");

    while (fgets(line, MAXLINE, fp_pipe) != NULL) {
        /* Remove the \n at the end of the line */

        line[strlen(line) - 1] = 0;

        /* Process the line */

        process_line(line, tree); 
    }

    pclose(fp_pipe);

    return tree;    
}

/**
**Funcio que es cridara quan creem els nostres fils, en aquesta funcio organitzarem
**els diferents pdfs que te cada fil.
**/
void *treeFils(void *arg){

    struct parametres *es = (struct parametres *) arg;//convertim amb un cast la nostra struct
    FILE *fp_pipe;
    int i,countLocal;
    countLocal = 0; 
    char command[MAXLINE],line[MAXLINE];
    
    
    while (count < es->num_elements) {
        
        //CountLocal sempre copia el count global. Es necesari ja que es pot donar el cas de que
        //mentre un fil procesi un fitxer, entri un altre i n'agafi el mateix. Es realitza entre
        //un mutex i un unlock ja que si entren dos fils "molt seguits" podrien iterar-lo dos vegades
        pthread_mutex_lock(&lock2);
        countLocal = count;
        count++;
        pthread_mutex_unlock(&lock2);

        //printf("Administrant: %s del Fil: %d\n",es->llistat_pdf[countLocal],es->fil);

        es->arbreLocal = create_tree(es->llistat_pdf[countLocal]);
        
        //fem que els altres fils no puguin continuar a partir daqui per poder modificar la dada global
        //arbreLocal de forma segura
        pthread_mutex_lock(&lock);

        copiarArbre(es->arbreGran,es->arbreLocal);//copiem larbre del fil a larbre global

        //desbloquejem el bloqueig que em fet abans ja que ja hem copiat l'arbre.
        pthread_mutex_unlock(&lock);


        /* AQUEST DELETE NO FUNCIONA I NO SABEM PER QUE */
        
        //deleteTree(es->arbreLocal); 
        free(es->arbreLocal);
    }

    free(es);
    free(fp_pipe);
}
//Funcio que creara tots els fils i administrara quin fil fa quins pdfs.
void crearfils(char **llista_pdfs,int num_pdfs,RBTree *arbrePrincipal){

    int i,j,err;
    struct parametres *estructura;
    //creem una struct per a cada fil, inicialitzem les seves variables, repartim els pdfs
    //entre tots els fils que volguem crear i creem finalment els fils.
    for(i = 0;i<NUMERO_FILS;i++){
        estructura = malloc(sizeof(struct parametres));
        estructura->num_elements = num_pdfs;
        estructura->llistat_pdf = llista_pdfs;
        estructura->fil = i;
        estructura->arbreGran = arbrePrincipal;

        err = pthread_create(&(ntid[i]),NULL,treeFils,(void *) estructura);

        if (err != 0) {
            printf("No puc crear el fil.\n");
        }

    }
}
//funcio encarregada de que els fils acabin.
void esperarfils(){
    int i,err2;
    struct parametres *estructura;
    for(i =0;i < NUMERO_FILS; i++){
        err2 = pthread_join(ntid[i],NULL);
        if (err2 != 0) {
            printf("Error pthread_join amb el fil\n");
        }
    }
    //destruim el mutex ja que els nostres fils ja han acabat.
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock2);
}

/**
 *
 *  Create the tree. This function reads the file filename and opens the pipe with pdftotext and calls process_line
 *  for each line that is received from it.
 *
 */
RBTree *admin_crearTree(char *filename)
{   
    FILE *fp, *fp_pipe;
    RBTree *tree;
    int i, num_pdfs;
    char line[MAXLINE];
    char **llista_pdfs;

	clock_t t1,t2;
	struct timeval tv1,tv2;

    /* Allocate memory for tree */
    tree = (RBTree *) malloc(sizeof(RBTree));

    /* Initialize the tree */
    initTree(tree);

    fp = fopen(filename, "r");
    if (!fp) {
        printf("ERROR: no he pogut obrir fitxer.\n");
        return NULL;
    }

    /* Llegim el fitxer. Suposem que el fitxer esta en un format correcte */
    fgets(line, MAXLINE, fp);
    num_pdfs = atoi(line);

    llista_pdfs = malloc(sizeof(char*)*num_pdfs);

    for(i = 0; i < num_pdfs; i++)
    {

        llista_pdfs[i] = malloc(sizeof(char)*MAXLINE);

        fgets(llista_pdfs[i], MAXLINE, fp);
        llista_pdfs[i][strlen(llista_pdfs[i])-1] = 0;
    }

    fclose(fp);

    //funcions que ens administraran els nostres fils.

	gettimeofday(&tv1,NULL);
	t1 = clock();

    crearfils(llista_pdfs,num_pdfs,tree);
    esperarfils();

    for(i = 0;i < num_pdfs;i++){
        free(llista_pdfs[i]);
    }

    free(llista_pdfs);
	gettimeofday(&tv2,NULL);
	t2 = clock();
    
    
    
	printf("Els fitxers han estat procesatts\n");
	printf("Temps de CPU: %f segons\n",(double)(t2-t1) / (double) CLOCKS_PER_SEC);
	printf("Temps real: %f segons\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));

	

    return tree;
}


/**
 *
 *  Save tree to disc
 *
 */

void save_tree_recursive(Node *x, FILE *fp)
{
    int i;

    if (x->right != NIL)
        save_tree_recursive(x->right, fp);

    if (x->left != NIL)
        save_tree_recursive(x->left, fp);

    i = strlen(x->data->key);
    fwrite(&i, sizeof(int), 1, fp);

    fwrite(x->data->key, sizeof(char), i, fp);

    i = x->data->num_vegades;
    fwrite(&i, sizeof(int), 1, fp);
}


void save_tree(RBTree *tree, char *filename)
{
    FILE *fp;

    int magic, nodes;

    fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: could not open file for writing.\n");
        return;
    }

    magic = MAGIC_NUMBER;
    fwrite(&magic, sizeof(int), 1, fp);

    nodes = count_nodes(tree);
    fwrite(&nodes, sizeof(int), 1, fp);

    save_tree_recursive(tree->root, fp);

    fclose(fp);  
}

/**
 *
 *  Load tree from disc
 *
 */

RBTree *load_tree(char *filename)
{
    FILE *fp;
    RBTree *tree;
    RBData *tree_data;

    int i, magic, nodes, len, num_vegades;
    char *paraula;

    fp = fopen(filename, "r");
    if (!fp) {
        printf("ERROR: could not open file for reading.\n");
        return NULL;
    }

    /* Read magic number */
    fread(&magic, sizeof(int), 1, fp);
    if (magic != MAGIC_NUMBER) {
        printf("ERROR: magic number is not correct.\n");
        return NULL;
    }

    /* Read number of nodes */
    fread(&nodes, sizeof(int), 1, fp);
    if (nodes <= 0) {
        printf("ERROR: number of nodes is zero or negative.\n");
        return NULL;
    }

    /* Allocate memory for tree */
    tree = (RBTree *) malloc(sizeof(RBTree));

    /* Initialize the tree */
    initTree(tree);

    /* Read all nodes. If an error is produced, the current read tree is
     * returned to the user. */
    for(i = 0; i < nodes; i++)
    {
        fread(&len, sizeof(int), 1, fp);
        if (len <= 0) {
            printf("ERROR: len is zero or negative. Not all tree could be read.\n");
            return tree;
        }

        paraula = malloc(sizeof(char) * (len + 1));
        fread(paraula, sizeof(char), len, fp);
        paraula[len] = 0;

        fread(&num_vegades, sizeof(int), 1, fp);
        if (num_vegades <= 0) {
            printf("ERROR: num_vegades is zero or negative. Not all tree could be read.\n");
            free(paraula);
            return tree;
        }

        tree_data = malloc(sizeof(RBData));

        tree_data->key = paraula;
        tree_data->num_vegades = num_vegades;

        insertNode(tree, tree_data); 
    }

    fclose(fp);

    return tree;
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

    printf("\n\nMenu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio d'un node\n");
    printf(" 5 - Sortir\n\n");
    printf("   Escull opcio: ");

    fgets(str, 5, stdin);
    opcio = atoi(str); 

    return opcio;
}

/** 
 *
 * Search the number of times a word appears in the tree
 *
 */

void search_word(RBTree *tree, char *word)
{
    RBData *tree_data;

    tree_data = findNode(tree, word);

    if (tree_data)
        printf("La paraula %s ha aparegut %d vegades als documents analitzats\n", word, tree_data->num_vegades);
    else
        printf("La paraula %s no apareix a l'arbre\n", word);
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

    RBTree *tree;

    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");

    /* Inicialitzem a un punter NULL */
    tree = NULL;

    do {
        opcio = menu();
        printf("\n\n");

        switch (opcio) {
            case 1:
                if (tree) {
                    printf("Alliberant arbre actual\n");
                    deleteTree(tree);
                    free(tree);
                }

                printf("Introdueix fitxer que conte llistat fitxers PDF: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;

                printf("Creant arbre...\n");
                tree = admin_crearTree(str);
                break;

            case 2:
                if (!tree) {
                    printf("No hi ha cap arbre a memoria\n");
                    break;
                }

                printf("Introdueix el nom de fitxer en el qual es desara l'arbre: ");
                fgets(str, MAXLINE, stdin);

                str[strlen(str)-1]=0;

                printf("Desant arbre...\n");
                save_tree(tree, str);
                break;

            case 3:
                if (tree) {
                    printf("Alliberant arbre actual\n");
                    deleteTree(tree);
                    free(tree);
                }

                printf("Introdueix nom del fitxer amb l'arbre: ");
                fgets(str, MAXLINE, stdin);

                str[strlen(str)-1]=0;

                printf("Llegint arbre de disc...\n");
                tree = load_tree(str);
                break;

            case 4:
                if (!tree) {
                    printf("No hi ha cap arbre a memoria\n");
                    break;
                }

                printf("Introdueix la paraula: ");
                fgets(str, MAXLINE, stdin);
                str[strlen(str)-1]=0;

                if (strlen(str) == 0) {
                    printf("No s'ha introduit cap paraula\n");
                    break;
                }

                search_word(tree, str);
                break;

            case 5:
                if (tree) {
                    printf("Alliberant memoria\n");
                    deleteTree(tree);
                    free(tree);
                }
                break;

            default:
                printf("Opcio no valida\n");
        } /* switch */
    }
    while (opcio != 5);

    return 0;
}



