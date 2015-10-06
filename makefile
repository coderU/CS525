pagerank: pagerank.c
	mpicc pagerank.c -o pagerank

clean:
	rm pagerank
