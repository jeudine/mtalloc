#include "mtalloc.h"
#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NB_ROUNDS 200
#define MAX_SIZE 10000000

pthread_t *threads;

mtm_t mtm;

static inline uint64_t hash64(uint64_t key) {
	key = (~key + (key << 21));
	key = key ^ key >> 24;
	key = ((key + (key << 3)) + (key << 8));
	key = key ^ key >> 14;
	key = ((key + (key << 2)) + (key << 4));
	key = key ^ key >> 28;
	key = (key + (key << 31));
	return key;
}

void *mt_routine(void *) {
	for (unsigned i = 0; i < NB_ROUNDS; i++) {
		const unsigned size     = rand() % MAX_SIZE;
		uint64_t *const in_buf  = (uint64_t *)mtmalloc(&mtm, size * sizeof(uint64_t));
		uint64_t *const out_buf = (uint64_t *)mtmalloc(&mtm, size * sizeof(uint64_t));
		for (unsigned j = 0; j < size; j++) {
			out_buf[j] = hash64(in_buf[j]);
		}
		mtfree(&mtm, in_buf);
		mtfree(&mtm, out_buf);
	}
	return (void *)NULL;
}

void *routine(void *) {
	for (unsigned i = 0; i < NB_ROUNDS; i++) {
		const unsigned size    = rand() % MAX_SIZE;
		uint64_t *const in_buf = (uint64_t *)malloc(size * sizeof(uint64_t));
		if (in_buf == NULL) {
			err(1, "%s:%d, malloc", __FILE__, __LINE__);
		}
		uint64_t *const out_buf = (uint64_t *)malloc(size * sizeof(uint64_t));
		if (out_buf == NULL) {
			err(1, "%s:%d, malloc", __FILE__, __LINE__);
		}
		for (unsigned j = 0; j < size; j++) {
			out_buf[j] = hash64(in_buf[j]);
		}
		free(in_buf);
		free(out_buf);
	}
	return (void *)NULL;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <number of threads>\n", argv[0]);
		exit(1);
	}

	const unsigned nb_threads = strtoul(argv[1], NULL, 10);

	threads = (pthread_t *)malloc(nb_threads * sizeof(pthread_t));
	if (threads == NULL) {
		err(1, "%s:%d, malloc", __FILE__, __LINE__);
	}

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (unsigned i = 0; i < nb_threads; i++) {
		if (pthread_create(&threads[i], NULL, routine, NULL)) {
			errx(1, "%s:%d, pthread_create", __FILE__, __LINE__);
		}
	}

	for (unsigned i = 0; i < nb_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			errx(1, "%s:%d, pthread_join", __FILE__, __LINE__);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Execution time without mtalloc: %f sec\n",
	       end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	mtm = mtm_init();
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (unsigned i = 0; i < nb_threads; i++) {
		if (pthread_create(&threads[i], NULL, mt_routine, NULL)) {
			errx(1, "%s:%d, pthread_create", __FILE__, __LINE__);
		}
	}

	for (unsigned i = 0; i < nb_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
			errx(1, "%s:%d, pthread_join", __FILE__, __LINE__);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Execution time with mtalloc: %f sec\n",
	       end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);
	mtm_destroy(mtm);
	free(threads);
	return 0;
}
