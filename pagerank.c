#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void sperate_by_space(int* array, char* line){
  char * pch;
  pch = strtok (line," ");
  int flag = 0;
  int count = 0;
  while (pch != NULL)
  {
    pch = strtok (NULL, " ");
    if(flag){
      *array = atof(pch);
      array++;
    }else{
      printf("%d: %s\n",count++, pch);

      flag = 1;
    }
  }
}

int main(int argc, char *argv[]) {
  int numprocs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int *val;
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
        count++;
      }
      printf("%d\n", count);
      switch (num) {
        case 0:
          printf("Need alloc %d for val\n", count-1 );
          val = (int *)malloc((count - 1 )*sizeof(int));
          sperate_by_space(val, line);
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
