#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef SMALLMATRIX
#define SMALLMATRIX 0
#endif

#ifndef POINT
#define POINT 0.0000001
#endif


float *val;
int *col;
int *row;
int l_val_size;
int size;
double *vector;
double* current_vector;
int iteration = 0;
int vector_size;
int single = 1;
int seperate = 16;
int complete = 0;
int clean_complete = 0;
int status = 0;
int global_change = 0;
int limit = 3000;
pthread_mutex_t complete_lock = PTHREAD_MUTEX_INITIALIZER;

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
void my_memcpy(double* dst, double* src, int size){
  int i = 0;
  for( i = 0 ; i < size ; i++){
    *(dst+i) = *(src+i);
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




double calculate_rank(double* vector, int row_number){
  int i= 0;
  double result = 0;
  int start = *(row+row_number);
  int end = *(row+row_number+1);
  //  fprintf(stderr,"%d %d\n",start,end);
  for(i = start ; i < end ; i++){
    result = result + (*(val+i))*(*(vector+*(col+i)));
  }
  return result;
}

void *calculate_rank_pthread(void *argument) {
  int i = 0;
  int index = *(int*)argument;
  fprintf(stderr,"%d thread start computing!\n",index);
  int change = 0;
  double compare;
  double difference;
  double segment = vector_size/(double)seperate;
  int self_complete = 0;
  int once = 0;
  while(1){

    if(status == 0){
      continue;
    }
    else if(status == -1){
      break;
    }else if(status == 2 && self_complete){
      self_complete = 0;
      change = 0;
      pthread_mutex_lock(&complete_lock);
      clean_complete++;
      pthread_mutex_unlock(&complete_lock);

    }else if(!self_complete && status ==1 ){
      //fprintf(stderr,"%d: %d-------%d\n",iteration, complete, index);
      for(i = (int)(index*(segment)) ; i <  (int)( (index+1)*(segment)) ; i++){
	compare = *(vector+i);	
	current_vector[i] = calculate_rank(vector,i);
	difference = compare - current_vector[i];
	if(change == 0 && (difference > POINT || (-1*difference)>POINT)){
	  change = 1;
	}
      }
      pthread_mutex_lock(&complete_lock);
      if(!change){
	global_change += 1;
      }
      complete++;
      pthread_mutex_unlock(&complete_lock);
      self_complete = 1;
    

      //Caculate page rank for self part
    }else{
      continue;
    }
  }
  return NULL;
}

int main(int argc, char *argv[]){
  struct timeval t1, t2;
  FILE * fp;
  char * line = NULL;
  size_t read;
  size_t len;
  int i, j;
  double **matrix;

  if(atoi(*(argv+2)) == 1){

    single = 1;
  }else{
    single = 0;
    seperate = atoi(*(argv+2));
    clean_complete = seperate;
  }
  
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

  int *temp = (int*)malloc(size*sizeof(int));
  for(i = 0 ; i < size ; i++){
    temp[i] = 0;
  }
  for(i = 0 ; i < l_val_size ; i++){
    temp[*(col+i)]++;
  }

  for(i = 0 ; i < l_val_size ; i++){
    *(val+i)= *(val+i)/temp[*(col+i)];
  }
  free(temp);

  
  if(SMALLMATRIX){
    printf("Initializing %d x %d matrix!\n",size - 1, size -1 );
    matrix = (double**)malloc((size-1)*sizeof(double*));
    for( i = 0 ; i < size - 1; i ++){
      *(matrix+i) = (double*)malloc((size-1)*sizeof(double));
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
  vector_size= size-1;
  vector = (double*)malloc(vector_size*sizeof(double));
  //Initilize vector
  
  for(i = 0 ; i < vector_size ; i++){
    vector[i] = 1/(double)vector_size;
  }
  fprintf(stderr,"Have %d nodes, and initial value is %G\n",vector_size, vector[0]);
  
  if(single){
    fprintf(stderr,"SINGLE PROCESS!\n");
    double* current = (double*)malloc(vector_size*sizeof(double));
    double compare;
    double difference;
    int change = 0;
    my_memcpy(current,vector,vector_size);
    gettimeofday(&t1, NULL);
    while(1){
      change = 0;
      for(i = 0 ; i < vector_size ; i++){
	compare = *(current+i);
	current[i] = calculate_rank(vector,i);
	difference = compare - current[i];
	//	fprintf(stderr,"%G\n",current[i]);
	if(difference > POINT || (-1*difference)>POINT){
	  change = 1;
	}
      }
      my_memcpy(vector,current,vector_size);
      if(!change){
	break;
      }
      if(SMALLMATRIX){
	for(i = 0 ; i < vector_size ; i++){
	  fprintf(stderr,"%G ",current[i]);
	}
	fprintf(stderr,"\n");
      }
      fprintf(stderr,"%d iteration complete\n",iteration);
      iteration++;
      if(iteration == limit){
	break;
      }
    }
    gettimeofday(&t2, NULL);

    FILE *f = fopen("pagerank.result.single", "w");
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
    
  }else{
    fprintf(stderr,"MUTI PROCESS!\n");
    current_vector = (double*)malloc(vector_size*sizeof(double));
    my_memcpy(current_vector,vector,vector_size);
    pthread_t thread[seperate];
    
    gettimeofday(&t1, NULL);
    
    for(i = 0 ; i < seperate ; i++){
      int *t = (int*)malloc(sizeof(int));
      *t = i;
      int iret = pthread_create( &thread[i], NULL, calculate_rank_pthread, (void*)(t));
      if(iret)
	{
	  fprintf(stderr,"Error - pthread_create() No.%d return code: %d\n",i, iret);
	  exit(EXIT_FAILURE);
	}
    }
    while(1){

      if(status != 1 && complete == 0 && global_change == 0){
	while(clean_complete != seperate){
	  
	}
	pthread_mutex_lock(&complete_lock);
	clean_complete = 0;
	pthread_mutex_unlock(&complete_lock);
	status = 1;
	//fprintf(stderr,"---------CLEAN--------\n");
      }
      else if(status != 2 && complete == seperate){
	status = 0;
	my_memcpy(vector,current_vector,vector_size);
	iteration++;
	if(global_change == seperate){
	  status = -1;
	  break;
	}
	pthread_mutex_lock(&complete_lock);
	complete = 0;
	global_change = 0;
	pthread_mutex_unlock(&complete_lock);
	status = 2;
	fprintf(stderr,"%d iteration complete %d\n",iteration,global_change);		
      }      
      if(iteration == limit){
	break;
      }


    }
    
    
    gettimeofday(&t2, NULL);

    FILE *f = fopen("pagerank.result.muti", "w");
    if (f == NULL)
    {
      printf("Error opening file!\n");
      exit(1);
    }
    fprintf(f, "time: %ld\n",  t2.tv_sec - t1.tv_sec);
    fprintf(f, "node_id | pagerank\n");
    for(i = 0 ; i < vector_size ; i++){
        fprintf(f,"%d | %f\n", i , *(vector+i));
    }
    fclose(f);
  }
  
}
