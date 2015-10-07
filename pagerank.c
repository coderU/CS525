#include <stdio.h>
#include <mpi.h>
int main(int argc, char *argv[]) {
  int numprocs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  char * pch;
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

    while ((read = getline(&line, &len, fp)) != -1) {
      printf("Retrieved line of length %zu :\n", read);
      pch = strtok (line ," ");
      while (pch != NULL)
      {
        // printf ("%s\n",pch);
        pch = strtok (NULL, " ");
      }
      printf('\n');
    }

    fclose(fp);
    if (line)
    free(line);
  }

  MPI_Finalize();
}
