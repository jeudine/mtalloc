#include "mtalloc.h"
#include <err.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define LOCK(mutex)                                                                                                    \
	{                                                                                                              \
		if (pthread_mutex_lock(&mutex)) {                                                                      \
			errx(1, "%s:%d, pthread_mutex_lock", __FILE__, __LINE__);                                      \
		}                                                                                                      \
	}

#define UNLOCK(mutex)                                                                                                  \
	{                                                                                                              \
		if (pthread_mutex_unlock(&mutex)) {                                                                    \
			errx(1, "%s:%d, pthread_mutex_unlock", __FILE__, __LINE__);                                    \
		}                                                                                                      \
	}

#define REALLOC(var, type, size)                                                                                       \
	{                                                                                                              \
		var = (type *)realloc(var, sizeof(type) * size);                                                       \
		if (var == NULL) {                                                                                     \
			err(1, "%s:%d, realloc", __FILE__, __LINE__);                                                  \
		}                                                                                                      \
	}

mtm_t mtm_init() {
	mtm_t mtm;
	memset(&mtm, 0, sizeof(mtm_t));
	pthread_mutex_init(&mtm.mut, NULL);
	return mtm;
}

void mtm_destroy(mtm_t mtm) {
	LOCK(mtm.mut);
	if (mtm.nb_used != 0) {
		errx(1, "mtm_destroy(): destroy a buffer while beign used");
	}
	for (unsigned i = 0; i < mtm.nb_units; i++) {
		free(mtm.buf[i]);
	}
	free(mtm.size);
	free(mtm.used);
	UNLOCK(mtm.mut);
	if (pthread_mutex_destroy(&mtm.mut)) {
		errx(1, "%s:%d, pthread_mutex_destroy", __FILE__, __LINE__);
	}
}

void *mtmalloc(mtm_t *const mtm, const size_t size) {
	if (size == 0) {
		return NULL;
	}
	void *out;
	LOCK(mtm->mut);
	if (mtm->nb_used == mtm->nb_units) {
		mtm->nb_units           = mtm->nb_units ? mtm->nb_units << 1 : 1;
		const unsigned nb_units = mtm->nb_units;
		REALLOC(mtm->buf, void *, nb_units);
		REALLOC(mtm->size, size_t, nb_units);
		REALLOC(mtm->used, int8_t, nb_units);
		const mtm_t _mtm        = *mtm;
		_mtm.size[mtm->nb_used] = size;
		_mtm.used[mtm->nb_used] = 1;
		for (unsigned i = _mtm.nb_used + 1; i < _mtm.nb_units; i++) {
			_mtm.buf[i]  = NULL;
			_mtm.size[i] = 0;
			_mtm.used[i] = 0;
		}

		out = malloc(size);
		if (out == NULL) {
			err(1, "%s:%d, malloc", __FILE__, __LINE__);
		}
		_mtm.buf[mtm->nb_used] = out;
		mtm->nb_used++;
	} else {
		mtm->nb_used++;
		const mtm_t _mtm      = *mtm;
		unsigned smallest_buf = 0;
		size_t size_smallest  = SIZE_MAX;
		unsigned good_buf;
		size_t size_good = SIZE_MAX;
		for (unsigned i = 0; i < _mtm.nb_units; i++) {
			if (_mtm.used[i] == 0) {
				if (_mtm.size[i] >= size) {
					if (_mtm.size[i] < size_good) {
						size_good = _mtm.size[i];
						good_buf  = i;
					}
				} else if (size_good == SIZE_MAX) {
					if (_mtm.size[i] < size_smallest) {
						size_smallest = _mtm.size[i];
						smallest_buf  = i;
					}
				}
			}
		}
		if (size_good != SIZE_MAX) {
			_mtm.used[good_buf] = 1;
			out                 = _mtm.buf[good_buf];
		} else {
			_mtm.used[smallest_buf] = 1;
			_mtm.size[smallest_buf] = size;
			free(_mtm.buf[smallest_buf]);
			out = malloc(size);
			if (out == NULL) {
				err(1, "%s:%d, malloc", __FILE__, __LINE__);
			}
			_mtm.buf[smallest_buf] = out;
		}
	}
	UNLOCK(mtm->mut);
	return out;
}

void *mtrealloc(mtm_t *const mtm, void *const ptr, size_t size) {
	if (size == 0) {
		mtfree(mtm, ptr);
		return NULL;
	}
	void *out  = NULL;
	mtm_t _mtm = *mtm;
	LOCK(_mtm.mut);
	size_t cur_size;
	unsigned cur_buf = _mtm.nb_units;
	for (unsigned i = 0; i < _mtm.nb_units; i++) {
		if (_mtm.buf[i] == ptr) {
			cur_size = _mtm.size[i];
			cur_buf  = i;
		}
	}

	if (cur_buf == _mtm.nb_units) {
		errx(1, "mtrealloc(): invalid pointer");
	}

	if (size <= cur_size) {
		out = ptr;
	} else {
		if (_mtm.nb_units != _mtm.nb_used) {
			unsigned good_buf;
			size_t size_good = SIZE_MAX;
			for (unsigned i = 0; i < _mtm.nb_units; i++) {
				if (_mtm.used[i] == 0 && _mtm.size[i] >= size && _mtm.size[i] < size_good) {
					size_good = _mtm.size[i];
					good_buf  = i;
				}
			}
			if (size_good != SIZE_MAX) {
				out = _mtm.buf[good_buf];
				memcpy(out, ptr, cur_size);
				_mtm.used[cur_buf]  = 0;
				_mtm.used[good_buf] = 1;
			}
		}
		if (out == NULL) {
			out = realloc(ptr, size);
			if (out == NULL) {
				err(1, "%s:%d, realloc", __FILE__, __LINE__);
			}
			_mtm.size[cur_buf] = size;
		}
	}
	UNLOCK(_mtm.mut);
	return out;
}

void *mtcalloc(mtm_t *const mtm, const size_t nmemb, const size_t size) {
	size_t _size = nmemb * size;
	if (_size == 0) {
		return NULL;
	}
	void *out = mtmalloc(mtm, _size);
	memset(out, 0, _size);
	return out;
}

void mtfree(mtm_t *const mtm, void *const ptr) {
	LOCK(mtm->mut);
	mtm->nb_used--;
	const mtm_t _mtm = *mtm;
	for (unsigned i = 0; i < _mtm.nb_units; i++) {
		if (_mtm.buf[i] == ptr) {
			_mtm.used[i] = 0;
			UNLOCK(mtm->mut);
			return;
		}
	}
	errx(1, "mtfree(): invalid pointer");
}
