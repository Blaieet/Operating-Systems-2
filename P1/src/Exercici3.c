#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2)
{
  double *num1, *num2;

  num1 = (double *) p1;
  num2 = (double *) p2;

  if (*num1 < *num2)
    return -1;
  if (*num1 > *num2)
    return 1;
  return 0;
}

int main(void)
{

	FILE *fp;
	int i, first_number;
	double *vector;
	char str[100];
	/* opening file for reading */
	fp = fopen("../dades/float.txt" , "r");
	if(fp == NULL) {
		perror("Error opening file");
		return(-1);
	}
	fgets(str, 100, fp);
	first_number = atoi(str);
	
	vector = malloc(sizeof(double) *first_number);
	for(i = 0; i < first_number; i++) {
		fgets(str, 100, fp);
		vector[i] = strtod(str, NULL);

	}

	qsort(vector, first_number, sizeof(double), compara);
	
	for(i = 0; i < first_number; i++) {
		printf("%lf,", vector[i]);
	}

	fclose(fp);
	return(0);
}
