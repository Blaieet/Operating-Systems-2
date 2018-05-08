#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compara(const void *p1, const void *p2)
{
	int l1,l2;
	char *str1, *str2;

 	str1 = *((char **) p1); 
	str2 = *((char **) p2);
	l1 = strlen(str1);
	l2 = strlen(str2);
	if (l1 > l2) {
		return 1;
	}
	if (l1 < l2) {
		return -1;
	}
	return 0;
}

int main(void)
{
  /*int i;
  char *vector[8] = {"hola", "sistemes", "operatius", "dos", "cadena", 
                     "quicksort", "canonada", "compara"};

  qsort(vector, 8, sizeof(char *), compara);

  printf("El vector ordenat és\n");

  for(i = 0; i < 8; i++)
    printf("%s\n", vector[i]);

  printf("\n");*/

	FILE *fp;
	int i, first_number,j;
	char **vector;
	char str[100];
	/* opening file for reading */
	fp = fopen("../dades/strings.txt" , "r");
	if(fp == NULL) {
		perror("Error opening file");
		return(-1);
	}
	fgets(str, 100, fp);
	first_number = atoi(str);
	vector = malloc(sizeof(char*) *first_number);
	for (i = 0; i < first_number; i++) {
		fgets(str, 100, fp);
		vector[i] = malloc(sizeof(char) *strlen(str));
		j = 0;
		while(str[j] != '\n') {
			vector[i][j] = str[j];
			j++;
		}
	}

	qsort(vector, first_number, sizeof(char*), compara);
	
	for(i = 0; i < first_number; i++) {
		printf("%s,", vector[i]);
		free(vector[i]);
	}
	
	fclose(fp);
	
	free(vector);

	return 0;
}
