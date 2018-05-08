#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2)
{
  int *num1, *num2;

  num1 = (int *) p1;
  num2 = (int *) p2;

  if (*num1 < *num2)
    return 1;
  if (*num1 > *num2)
    return -1;
  return 0;
}



int main(void)
{
  /*int i;
  int vector[8] = {8, 4, 2, 3, 5, 6, 8, 5};

  qsort(vector, 8, sizeof(int), compara);

  printf("El vector ordenat és ");

  for(i = 0; i < 8; i++)
    printf("%d ", vector[i]);
  
  printf("\n");*/

	FILE *fp;
	int i, first_number;
	int *vector;
	char str[100];
	/* opening file for reading */
	fp = fopen("../dades/integers.txt" , "r");
	if(fp == NULL) {
		perror("Error opening file");
		return(-1);
	}
	fgets(str, 100, fp);
	first_number = atoi(str);
	
	vector = malloc(sizeof(int) *first_number);
	for(i = 0; i < first_number; i++) {
		fgets(str, 100, fp);
		vector[i] = atoi(str);

	}

	qsort(vector, first_number, sizeof(int), compara);
	
	for(i = 0; i < first_number; i++) {
		printf("%d,", vector[i]);
	}

	fclose(fp);
	return(0);
}
