pagerank: pagerank.c
	mpicc pagerank.c -lpthread -o pagerank

clean:
	rm pagerank
