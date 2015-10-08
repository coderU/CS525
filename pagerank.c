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
  int max = 0;
  int size;
  int l_size;
  int l_val_size;
  int i, j;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);
  float** matrix;
  float** part_val;
  int** part_col;
  int** part_row;
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
        val = (float *)malloc((count - 1)*sizeof(float));
        sperate_by_space_f(val, line);
        if(DEBUG){
          printf("Need alloc %d for val\n", count-1 );
          printf("%d %f\n",(count -2), *(val+count-2));
        }
        l_val_size = count - 1;
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
    //READ PARTITION
    len = 0;
    fp = fopen("./graphs/200K-graph.txt.part.4", "r");
    if (fp == NULL){
      return 0;
    }
    part = (int*)malloc(size*sizeof(int));
    int part_count = 0;
    int current = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
      if(*line == '\n'){
        break;
      }
      current = atoi(line);
      max = (max > current)?max:current;
      *(part+part_count) = atoi(line);
      part_count++;
    }
    if(DEBUG){
      printf("%d %d %d\n", *(part+part_count-1),part_count-1,max);
    }
    fclose(fp);
    if (line){
      free(line);
    }
    //**************************************************************************
    //Seperate NODES
    // int final_count[(max+1)];
    // for(i = 0 ; i < (max+1) ; i++){
    //   final_count[i] = 0;
    // }
    // for(i = 0 ; i < part_count ; i++){
    //   final_count[*(part+i)]++;
    // }
    // if(DEBUG){
    //   for(i = 0 ; i < (max+1) ; i++){
    //     printf("%d: %d\n",i, final_count[i]);
    //   }
    // }
    // part_val = (float**)malloc((max+1)*sizeof(float*));
    // part_col = (int**)malloc((max+1)*sizeof(int*));
    // part_row = (int**)malloc((max+1)*sizeof(int*));
    //
    // for(i = 0 ; i < (max+1) ; i++){
    //   *(part_val+i) = (float*)malloc(final_count[i]*sizeof(float));
    //   *(part_col+i) = (int*)malloc(final_count[i]*sizeof(int));
    //   *(part_row+i) = (int*)malloc(final_count[i]*sizeof(int));
    // }
    //
    // for(i = 0 ; i < part_count ; i++){
    //   int temp_row =
    // }
  }
  MPI_Bcast(&l_val_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(rank != 0){
    val = (float *)malloc(l_val_size*sizeof(float));
  }
  MPI_Bcast(val, l_val_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

  if(rank != 0){
    col = (int *)malloc(l_val_size*sizeof(int));
  }
  MPI_Bcast(col, l_val_size, MPI_INT, 0, MPI_COMM_WORLD);


  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(rank != 0){
    row = (int *)malloc(size*sizeof(int));
  }
  MPI_Bcast(row, size, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);

  if(DEBUG){
    printf("%d: %d %f\n",rank, l_val_size -1, *(val+l_val_size-1));
    printf("%d: %d %d\n",rank, l_val_size -1, *(col+l_val_size-1));
    printf("%d: %d %d\n",rank, size -1, *(row+size-1));
    printf("**************************************************************************\n");
  }
  //**************************************************************************
  //CREATE MATRIX
  matrix = (float**)malloc((size-1)*sizeof(float*));
  for( i = 0 ; i < size - 1; i ++){
    *matrix = (float*)malloc((size-1)*sizeof(float));
  }

  for(i = 0 ; i < size - 1 ; i++){
    for(j = 0 ; j < size - 1 ; j++){
      printf("%f \n", *(*(matrix+i)+j));
      // *(*(matrix+i)+j) = 0;
    }
    printf("\n");
  }
  // for(i = 0 ; i < size - 1 ; i++){
  //   int start = *(row+i);
  //   int end = *(row+i+1);
  //   for(j = start ; j < end ; j++){
  //     *(*(matrix+i)+*(col+j)) = *(val+j);
  //   }
  // }

  MPI_Finalize();
}
