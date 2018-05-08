#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2)
{
  float *num1, *num2;

  num1 = (float *) p1;
  num2 = (float *) p2;

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
	float *vector;
	char str[100];
	/* opening file for reading */
	fp = fopen("../dades/float.txt" , "r");
	if(fp == NULL) {
		perror("Error opening file");
		return(-1);
	}
	fgets(str, 100, fp);
	first_number = atoi(str);
	
	vector = malloc(sizeof(float) *first_number);
	for(i = 0; i < first_number; i++) {
		fgets(str, 100, fp);
		vector[i] = atof(str);
	}

	qsort(vector, first_number, sizeof(float), compara);
	
	for(i = 0; i < first_number; i++) {
		printf("%f,", vector[i]);
	}

	fclose(fp);
	return(0);
}
