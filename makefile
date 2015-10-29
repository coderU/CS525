pagerank: pagerank_thread.c
	gcc -pthread pagerank_thread.c -o pagerank

clean:
	rm pagerank
