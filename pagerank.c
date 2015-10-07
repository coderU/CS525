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
        *(array++) = atof(str);
      }
      strcpy(str,"");
      j=0;
      count++;
    }
    i++;
  }
}


void sperate_by_space_i(int* array, char* line){
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
        *(array++) = atoi(str);
        // printf("%d\n", *array);
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
  int *part;
  int size;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);
  float** matrix;
  if(rank == 0){
    //**************************************************************************
    //READ CRS
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
        val = (float *)malloc((count - 1)*sizeof(int));
        sperate_by_space_f(val, line);
        if(DEBUG){
          printf("Need alloc %d for val\n", count-1 );
          printf("%d %f\n",(count -2), *(val+count-2));
        }
        break;
        case 1:
        col = (int *)malloc((count - 1)*sizeof(int));
        sperate_by_space_i(col, line);
        if(DEBUG){
          printf("Need alloc %d for col\n", count-1 );
          printf("%d %d\n",(count -2), *(col+count-2));
        }
        break;
        case 2:
        row = (int *)malloc((count - 1)*sizeof(int));
        sperate_by_space_i(row, line);
        if(DEBUG){
          printf("Need alloc %d for row\n", count-1 );
          printf("%d %d\n",(count -2), *(row+count-2));
        }
        size = count-1;
        break;
      }
      num++;
    }
    fclose(fp);
    if (line){
      free(line);
    }
    //**************************************************************************
    fp = fopen("./graphs/200K-graph.txt.part.4", "r");
    if (fp == NULL){
      return 0;
    }
    part = (int*)malloc(size*sizeof(int));
    int part_count = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
      if(*line == '\n'){
        break;
      }
      *(part+part_count) = atoi(line);
      part_count++;
    }
    printf("%d\n", *(part+part_count-1));
    fclose(fp);
    if (line){
      free(line);
    }
  }

  MPI_Finalize();
}
