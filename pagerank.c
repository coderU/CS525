#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG 1
#endif
void sperate_by_space_f(float* array, char* line){
  int i=0,j=0, flag = 0;
  char str[10];
  int count = 0;
  while(line[i]!='\0')
  {
    if(line[i]!=' ')
    {
      str[j++]=line[i];
    }
    else
    {
      str[j]='\0';
      if(!flag){
        flag = 1;
      }else{
        *array = atof(str);
      }
      strcpy(str,"");
      j=0;
      count++;
    }
    i++;
  }
}

int main(int argc, char *argv[]) {
  int numprocs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  float *val;
  int *col;
  int *row;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);

  if(rank == 0){

    fp = fopen("./graphs/200K-graph.txt", "r");
    if (fp == NULL){
      return 0;
    }
    int num = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
      int count = 0;
      char* temp = line;
      while(*temp != '\0'){
        temp++;
        if(*temp == ' '){
          count++;
        }
      }
      printf("%d\n", count);
      switch (num) {
        case 0:
        printf("Need alloc %d for val\n", count-1 );
        val = (float *)malloc((count - 1 )*sizeof(int));
        sperate_by_space_f(val, line);
        printf("%d %f\n",(count -1), *(val+count-3));
        break;
        case 1:
        col = (int *)malloc((count - 1 )*sizeof(int));
        printf("Need alloc %d for col\n", count-1 );
        break;
        case 2:
        row = (int *)malloc((count - 1 )*sizeof(int));
        printf("Need alloc %d for row\n", count-1 );
        break;
      }

      num++;
    }

    fclose(fp);
    if (line){
      free(line);
    }
  }

  MPI_Finalize();
}
