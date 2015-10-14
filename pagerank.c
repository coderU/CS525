#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef SMALLMATRIX
#define SMALLMATRIX 0
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

void compare_vector(float* a, float* b, int size, int rank){
  int i = 0 ;
  for(i = 0 ; i < size ; i++){
    if(*(a+i) != *(b+i)){
      printf("Process %d has Node %d different: Correct: %f Current: %f\n",rank, i , *(a+i),*(b+i) );
      return;
    }
  }
  printf("Process %d has the same vectors\n", rank);
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

float calculate_rank(float* val, int* col, int* row, int node_index, float* vector, int flag){
  float sum = 0;
  int start = *(row+node_index);
  int end = *(row+node_index+1);
  int i = 0;
  for( i = start ; i < end ; i++){
    sum = sum + (*(val+i))*(*(vector+*(col+i)));
  }

  return sum;

}

void combine_vector(float* a, float* b, int size){
  int i = 0;
  for( i = 0 ; i < size ; i++){
    *(a+i) = *(a+i)+*(b+i);
  }
}

int calculate_diff(float* a, float* b, int size){
  int i = 0;
  for( i = 0 ; i < size ; i++){
    if((*(a+i)-*(b+i))>0.00001 || (*(a+i)-*(b+i))<-0.00001){
      return 0;
    }
  }
  return 1;
}

void my_memcpy(float* dst, float* src, int size){
  int i = 0;
  for( i = 0 ; i < size ; i++){
    *(dst+i) = *(src+i);
  }
}
void print_vector(float* a, int size){
  if(SMALLMATRIX){
    int i = 0 ;
    for( i = 0 ; i < size ; i++){
      printf("%dth node of vector have value: %f\n", i , *(a+i) );
    }
  }
}

void print_vector_t(float* a, int size){
    int i = 0 ;
    for( i = 0 ; i < size ; i++){
      if(*(a+i)> 0){
        printf("%dth node of vector have value: %f\n", i , *(a+i) );
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
  float *val;
  int *col;
  int *row;
  int *part;
  int max = 0;
  int size;
  int l_size;
  int l_val_size;
  int i, j, k;
  int part_count = 0;
  int *subgraph_count;
  int **subgraph;
  int *subgraph_index;
  float* send;
  struct timeval t1, t2;
  int neccessery_count = 0;
  int *l_neccessery;
  int* root_neccessery_count;
  int** root_neccessery;
  float *neccessery_value;
  int local_subgraph_count = 0;
  int* local_subgraph;
  float* local_subgraph_vector;
  if(argc != 3){
    printf("USAGE: mpirun -machinefile machines -np *Number of partition* pagerank *graph-file* *graph-partition-file*\n");
    if(DEBUG){
      printf("GRAPH-FILE: %s\n", *(argv+1));
      printf("GRAPH-PARTITION-FILE: %s\n", *(argv+2));
    }
    exit(-1);
  }
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
    fp = fopen(*(argv+1), "r");
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
    fp = fopen(*(argv+2), "r");
    if (fp == NULL){
      return 0;
    }
    part = (int*)malloc(size*sizeof(int));
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

    subgraph_count = (int*)malloc((max+1)*sizeof(int));
    subgraph_index = (int*)malloc((max+1)*sizeof(int));
    subgraph = (int**)malloc((max+1)*sizeof(int*));
    for(i = 0 ; i < (max+1) ; i++){
      *(subgraph_count+i)=0;
      *(subgraph_index+i)=0;
    }

    for(i = 0 ; i < part_count ; i++){
      *(subgraph_count+*(part+i)) = *(subgraph_count+*(part+i))+1;
    }


    for(i = 0 ; i < (max+1) ; i++){
      printf("Node %d has: %d elements\n",i, *(subgraph_count+i) );
    }
    for(i = 0 ; i < (max+1) ; i++){
      *(subgraph+i)=(int*)malloc(*(subgraph_count+i)*sizeof(int));
      for(j = 0 ; j < *(subgraph_count+i) ; j++){
        *(*(subgraph+i)+j)=0;
      }
    }

    for(i = 0 ; i < part_count ; i++){
      int index = *(part+i);
      *(*(subgraph+index)+*(subgraph_index+index)) = i;
      *(subgraph_index+index) = *(subgraph_index+index) + 1;
    }

    // int temp[size-1];
    int *temp = (int*)malloc((size-1)*sizeof(int));
    for( i = 0 ; i < size -1 ; i++){
      temp[i] = 0;
    }
    for(i = 0 ; i < l_val_size ; i++){
      temp[*(col+i)] = temp[*(col+i)]+1;
    }
    for(i = 0 ; i < l_val_size; i++){
      *(val+i) = *(val+i)/temp[*(col+i)];
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
  MPI_Bcast(&part_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(rank != 0){
    part = (int*)malloc(part_count*sizeof(int));
  }

  MPI_Bcast(part, part_count, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  if(DEBUG){
    printf("%d: %d %f\n",rank, l_val_size -1, *(val+l_val_size-1));
    printf("%d: %d %d\n",rank, l_val_size -1, *(col+l_val_size-1));
    printf("%d: %d %d\n",rank, size -1, *(row+size-1));
    printf("%d: %d %d\n",rank, part_count -1, *(part+part_count-1));
    printf("**************************************************************************\n");
  }
  //**************************************************************************
  //CREATE MATRIX
  if(rank == 0 && SMALLMATRIX){
    printf("Initializing %d x %d matrix!\n",size - 1, size -1 );
    matrix = (float**)malloc((size-1)*sizeof(float*));
    for( i = 0 ; i < size - 1; i ++){
      *(matrix+i) = (float*)malloc((size-1)*sizeof(float));
      for( j = 0 ; j < size - 1 ; j++){
        *(*(matrix+i)+j) = 0;

      }
    }

    for(i = 0 ; i < size - 1 ; i++){
      int start = *(row+i);
      int end = *(row+i+1);
      for(j = start ; j < end ; j++){
        *(*(matrix+i)+*(col+j)) = *(val+j);
      }
    }

    for( i = 0 ; i < size -1 ; i++){
      for( j = 0 ; j < size -1 ; j++){
        printf("%f ", *(*(matrix+i)+j));
      }
      printf("\n" );
    }
  }

  float *vector = (float*)malloc((size-1)*sizeof(float));
  float *t_vector = (float*)malloc((size-1)*sizeof(float));
  float *l_vector = (float*)malloc((size-1)*sizeof(float));

  int* index;
  for(i = 0 ; i < size-1; i++){
    *(vector+i) = 0;
  }
  *vector = 1;
  int elements_count;
  if(rank == 0){
    if(DEBUG){
      print_vector(vector, size-1);
    }
    //DISTRIBUTE ALL NECCESSERY VECTOR ELEMENTS
    root_neccessery_count = (int*)malloc((max+1)*sizeof(int));
    root_neccessery = (int**)malloc((max+1)*sizeof(int*));
    for(i = 1 ; i < (max+1) ; i++){
      MPI_Send((subgraph_count+i), 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(*(subgraph+i), *(subgraph_count+i), MPI_INT, i, 0, MPI_COMM_WORLD);

      //TODO: SEND ONLY NECCESSERY
      // int temp_array[size - 1];
      int *temp_array = (int*)malloc((size-1)*sizeof(int));
      for(j = 0 ; j < size -1 ; j++){
        temp_array[j] = 0;
      }

      for(j = 0 ; j < *(subgraph_count + i); j++){
        int start = *(row+ *(*(subgraph+i)+j) );
        int end = *(row+ *(*(subgraph+i)+j) + 1);

        for(k = start ; k < end ; k++){
          temp_array[*(col+k)] = 1;
        }
      }

      neccessery_count = 0;
      for( j = 0 ; j < size -1 ; j++){
        if(temp_array[j] != 0){
          neccessery_count++;
        }
      }

      int neccessery[neccessery_count];
      // int* neccessery = (int*)malloc(neccessery_count*sizeof(neccessery_count));
      int neccessery_index = 0;
      for( j = 0 ; j < size -1 ; j++){
        if(temp_array[j] != 0){
          neccessery[neccessery_index++] = j;
        }
      }
      if(DEBUG){
        printf("SEND-----Process: %d ONLY NEED %d ELEMENTS FROM THE VECTOR\n", i, neccessery_count);
        printf("SEND-----Process: %d WILL HAVE LAST NECCESSERY ELEMENT: %d\n",i, neccessery[neccessery_count-1] );
      }
      *(root_neccessery_count+i) = neccessery_count;
      *(root_neccessery+i) = (int*)malloc(neccessery_count*sizeof(int));
      for(j = 0 ; j < neccessery_count ; j++){
        *(*(root_neccessery+i)+j)=neccessery[j];
      }
      MPI_Send(&neccessery_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&neccessery, neccessery_count, MPI_INT, i, 0, MPI_COMM_WORLD);
      // float temp_value[neccessery_count];
      // for(j = 0 ; j < neccessery_count ; j++){
      //   temp_value[j] = *(vector+neccessery[j]);
      // }
      // MPI_Send(&temp_value, neccessery_count, MPI_FLOAT, i, 0, MPI_COMM_WORLD);

      //********************************
      MPI_Send((subgraph_count+i), 1 ,MPI_INT, i , 0 , MPI_COMM_WORLD);

      MPI_Send( *(subgraph+i), *(subgraph_count+i) ,MPI_INT, i , 0 , MPI_COMM_WORLD);

      MPI_Send(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(vector, (size-1), MPI_FLOAT, i, 0, MPI_COMM_WORLD);
    }
    for(i = 0 ; i < size -1 ; i++){
      *(l_vector+i)=0;
    }
    for( i = 0 ; i < *subgraph_count ; i++){
      int node_index = *(*(subgraph+rank)+i);
      float value = calculate_rank(val, col, row, node_index, vector,1);
      *(l_vector+*(*(subgraph+rank)+i)) = value;
    }


    for(i = 1 ; i < (max+1) ; i++){
      MPI_Recv(t_vector, (size-1), MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      float* origin = (float*)malloc(*(subgraph_count+i)*sizeof(float));
      MPI_Recv(origin, *(subgraph_count+i), MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      float* temp_vector = (float*)malloc((size-1)*sizeof(float));
      for( j = 0 ; j < (size-1) ; j++){
        *(temp_vector+j) = 0;
      }
      for( j = 0 ; j < *(subgraph_count+i) ; j++){
        *(temp_vector+*(*(subgraph+i)+j)) = *(origin+j);
      }

      compare_vector(temp_vector, t_vector,*(subgraph_count+i),i);
      combine_vector(l_vector, t_vector, size-1);
    }
    my_memcpy(vector,l_vector,size-1);
    if(DEBUG){
      if(SMALLMATRIX){
        printf("*******************---0---*************************\n" );
      }
      print_vector(vector, size-1);
    }
  }
  else{
    MPI_Recv(&elements_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    index = (int*)malloc(elements_count*sizeof(int));
    MPI_Recv(index, elements_count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Recv(&neccessery_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    l_neccessery = (int*)malloc(neccessery_count*sizeof(int));
    MPI_Recv(l_neccessery, neccessery_count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    neccessery_value = (float*)malloc(neccessery_count*sizeof(int));
    MPI_Recv(&local_subgraph_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local_subgraph = (int*)malloc(local_subgraph_count*sizeof(int));
    local_subgraph_vector = (float*)malloc(local_subgraph_count*sizeof(int));
    MPI_Recv(local_subgraph, local_subgraph_count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // fprintf(stderr, "aaaa\n" );
    // MPI_Recv(neccessery_value, neccessery_count, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // float neccessery_vector[size-1];
    // for(i = 0 ; i < (size -1 ); i++){
    //   neccessery_vector[i] = 0;
    // }
    // for( i = 0 ; i < neccessery_count ; i++){
    //   neccessery_vector[*(l_neccessery+i)] = *(neccessery_value+i);
    // }

    if(DEBUG){
      printf("RCV-----Process: %d ONLY NEED %d ELEMENTS FROM THE VECTOR\n", rank, neccessery_count);
      printf("RCV-----Process: %d WILL HAVE LAST NECCESSERY ELEMENT: %d\n",rank, l_neccessery[neccessery_count-1] );
    }
    MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(vector, (size-1), MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // if(DEBUG){
    //   compare_vector(vector, neccessery_vector,size-1,rank);
    // }
    for(i = 0 ; i < size -1 ; i++){
      *(l_vector+i)=0;
    }
    // if(DEBUG){
    //   printf("After Distributed node %d has %d elements and last element should be %d\n", rank, elements_count,*(index+elements_count-1));
    //   printf("After Distributed local vector has %d nodes and first one is %f while last one is %f\n",size-1, *vector, *(vector+size-2) );
    // }
    for( i = 0 ; i < elements_count ; i++){
      int node_index = *(index+i);
      float value = calculate_rank(val, col, row, node_index, vector,1);
      *(l_vector+*(index+i)) = value;
      // fprintf(stderr, "a: %d\n", *(index+i));
    }
    MPI_Send(l_vector, (size-1), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

    for( i = 0 ; i < local_subgraph_count ; i++){
      *(local_subgraph_vector+i) = *(l_vector+*(local_subgraph+i));
    }
    MPI_Send(local_subgraph_vector, (local_subgraph_count), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  printf("------------------------Start Computing!----------------------------\n" );

  if(rank == 0){
    gettimeofday(&t1, NULL);
  }
  int iteration = 1;
  int ok = 0;
  while(1 < 3){
    if(rank == 0){
      //DISTRIBUTE ALL NECCESSERY VECTOR ELEMENTS
      for(i = 1 ; i < (max+1) ; i++){
        //TODO: SEND ONLY NECCESSERY
        neccessery_count = *(root_neccessery_count+i);

        float temp_value[neccessery_count];
        // float *temp_value = (float*)malloc(neccessery_count*sizeof(float));
        for(j = 0 ; j < neccessery_count ; j++){
          temp_value[j] = *(vector+*(*(root_neccessery+i)+j));
        }
        MPI_Send(&temp_value, neccessery_count, MPI_FLOAT, i, 0, MPI_COMM_WORLD);

        //***************
        // MPI_Send(vector, (size-1), MPI_FLOAT, i, 0, MPI_COMM_WORLD);
      }

      for(i = 0 ; i < size -1 ; i++){
        *(l_vector+i)=0;
      }

      for( i = 0 ; i < *subgraph_count ; i++){
        int node_index = *(*(subgraph+rank)+i);
        float value = calculate_rank(val, col, row, node_index, vector,0);
        *(l_vector+*(*(subgraph+rank)+i)) = value;
      }

      for(i = 1 ; i < (max+1) ; i++){
        MPI_Recv(t_vector, (size-1), MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        float* origin = (float*)malloc(*(subgraph_count+i)*sizeof(float));
        MPI_Recv(origin, *(subgraph_count+i), MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        float* temp_vector = (float*)malloc((size-1)*sizeof(float));
        for( j = 0 ; j < (size-1) ; j++){
          *(temp_vector+j) = 0;
        }
        for( j = 0 ; j < *(subgraph_count+i) ; j++){
          *(temp_vector+*(*(subgraph+i)+j)) = *(origin+j);
        }

        combine_vector(l_vector, t_vector, size-1);
      }

      ok = calculate_diff(vector,l_vector, size-1);
      my_memcpy(vector,l_vector,size-1);
      if(DEBUG){
        if(SMALLMATRIX){
          printf("*******************---%d---*************************\n", iteration);
        }
        print_vector(vector, size-1);
      }

    }
    else{
      MPI_Recv(neccessery_value, neccessery_count, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // float neccessery_vector[size-1];
      float *neccessery_vector = (float*)malloc((size-1)*sizeof(float));
      for(i = 0 ; i < (size -1 ); i++){
        neccessery_vector[i] = 0;
      }
      for( i = 0 ; i < neccessery_count ; i++){
        neccessery_vector[*(l_neccessery+i)] = *(neccessery_value+i);
      }
      // MPI_Recv(vector, (size-1), MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


      for( i = 0 ; i < size -1 ; i++){
        *(vector+i) = neccessery_vector[i];
      }
      // printf("TEST: %f %f\n", *(vector+616), neccessery_vector[616]);
      for(i = 0 ; i < size -1 ; i++){
        *(l_vector+i)=0;
      }
      for( i = 0 ; i < elements_count ; i++){
        int node_index = *(index+i);
        float value = calculate_rank(val, col, row, node_index, vector,0);
        *(l_vector+*(index+i)) = value;
      }
      MPI_Send(l_vector, (size-1), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
      for( i = 0 ; i < local_subgraph_count ; i++){
        *(local_subgraph_vector+i) = *(l_vector+*(local_subgraph+i));
      }
      MPI_Send(local_subgraph_vector, (local_subgraph_count), MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Bcast(&ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0){
      printf("Interation %d Complete!\n", iteration);
    }
    iteration++;
    MPI_Barrier(MPI_COMM_WORLD);
    if(ok){
      break;
    }
  }

  if(rank == 0){
    gettimeofday(&t2, NULL);
    if(DEBUG){
      for(i = 0 ; i < size-1 ; i++){
        // if(*(vector+i)>0.00001)
        if(i <= 10 && i >= 0)
          printf("After %d iteration vector node: %d has value %f \n", iteration, i , *(vector+i));
      }
    }
    printf("Total Time Cost: %ld secs\n", t2.tv_sec - t1.tv_sec);
    FILE *f = fopen("pagerank.result", "w");
    if (f == NULL)
    {
      printf("Error opening file!\n");
      exit(1);
    }
    fprintf(f, "time: %ld\n",  t2.tv_sec - t1.tv_sec);
    fprintf(f, "node_id | pagerank\n");
    for(i = 0 ; i < size-1 ; i++){
        fprintf(f,"%d | %f\n", i , *(vector+i));
    }
    fclose(f);
  }
  MPI_Finalize();
}
