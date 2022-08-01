#ifndef MTALLOC_H
#define MTALLOC_H
#include <pthread.h>
#include <stdint.h>

typedef struct {
	void **buf;
	unsigned nb_units;
	unsigned nb_used;
	size_t *size;
	int8_t *used;
	pthread_mutex_t mut;
} mtm_t;

void *mtmalloc(mtm_t *mtm, size_t size);
void *mtrealloc(mtm_t *mtm, void *ptr, size_t size);
void *mtcalloc(mtm_t *mtm, size_t nmemb, size_t size);
void mtfree(mtm_t *mtm, void *ptr);

mtm_t mtm_init();
void mtm_destroy(mtm_t mtm);

#endif
