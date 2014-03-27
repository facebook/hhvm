/*
   +----------------------------------------------------------------------+
   | Thread Safe Resource Manager                                         |
   +----------------------------------------------------------------------+
   | Copyright (c) 1999-2011, Andi Gutmans, Sascha Schumann, Zeev Suraski |
   | This source file is subject to the TSRM license, that is bundled     |
   | with this package in the file LICENSE                                |
   +----------------------------------------------------------------------+
   | Authors:  Zeev Suraski <zeev@zend.com>                               |
   +----------------------------------------------------------------------+
*/
#include <stdlib.h>
#include <pthread.h>
#include "TSRM.h"

struct tsrm_resource_type {
	tsrm_resource_type() {}

	size_t size;
	ts_allocate_ctor ctor;
	ts_allocate_dtor dtor;
	int done;
};

static std::vector<tsrm_resource_type> resource_types_table;

HPHP::ThreadLocal<TSRMStorageVector> tsrm_thread_resources;

#define TSRM_ERROR(args)

TSRM_API ts_rsrc_id ts_allocate_id(ts_rsrc_id *rsrc_id, size_t size, ts_allocate_ctor ctor, ts_allocate_dtor dtor)
{
	TSRM_ERROR((TSRM_ERROR_LEVEL_CORE, "Obtaining a new resource id, %d bytes", size));

	/* obtain a resource id */
	resource_types_table.resize(resource_types_table.size() + 1);
	*rsrc_id = TSRM_SHUFFLE_RSRC_ID(resource_types_table.size() - 1);
	TSRM_ERROR((TSRM_ERROR_LEVEL_CORE, "Obtained resource id %d", *rsrc_id));

	/* store the new resource type in the resource sizes table */
	resource_types_table[TSRM_UNSHUFFLE_RSRC_ID(*rsrc_id)].size = size;
	resource_types_table[TSRM_UNSHUFFLE_RSRC_ID(*rsrc_id)].ctor = ctor;
	resource_types_table[TSRM_UNSHUFFLE_RSRC_ID(*rsrc_id)].dtor = dtor;
	resource_types_table[TSRM_UNSHUFFLE_RSRC_ID(*rsrc_id)].done = 0;

	TSRM_ERROR((TSRM_ERROR_LEVEL_CORE, "Successfully allocated new resource id %d", *rsrc_id));
	return *rsrc_id;
}

void * ts_init_resource(int id)
{
	assert(id != 0);
	TSRMLS_FETCH();
	TSRMStorageVector & vec = *tsrm_thread_resources;
	if (vec.size() <= TSRM_UNSHUFFLE_RSRC_ID(id)) {
		vec.resize(TSRM_UNSHUFFLE_RSRC_ID(id) + 1);
	}
	tsrm_resource_type & type = resource_types_table.at(TSRM_UNSHUFFLE_RSRC_ID(id));
	if (vec[TSRM_UNSHUFFLE_RSRC_ID(id)] == nullptr) {
		vec[TSRM_UNSHUFFLE_RSRC_ID(id)] = malloc(type.size);
		if (type.ctor) {
			type.ctor(vec[TSRM_UNSHUFFLE_RSRC_ID(id)] TSRMLS_CC);
		}
	}
	return vec[TSRM_UNSHUFFLE_RSRC_ID(id)];
}

void ts_free_id(ts_rsrc_id id)
{
	TSRMStorageVector & vec = *tsrm_thread_resources;
	int j = TSRM_UNSHUFFLE_RSRC_ID(id);
	assert(id != 0);
	TSRMLS_FETCH();
	if (vec.size() <= j && vec[j] != nullptr) {
		if (resource_types_table[j].dtor) {
			resource_types_table[j].dtor(vec[j] TSRMLS_CC);
		}
		free(vec[j]);
	}
}

// [HHVM] We implement a few utility functions here, using the same
// implementation as in PHP, except with the non-pthread implementations
// removed.

/* Obtain the current thread id */
TSRM_API THREAD_T tsrm_thread_id(void)
{
	return pthread_self();
}

/* Allocate a mutex */
TSRM_API MUTEX_T tsrm_mutex_alloc(void)
{
	MUTEX_T mutexp;
	mutexp = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutexp,NULL);
	return( mutexp );
}

/* Free a mutex */
TSRM_API void tsrm_mutex_free(MUTEX_T mutexp)
{
	if (mutexp) {
		pthread_mutex_destroy(mutexp);
		free(mutexp);
	}
}

/*
  Lock a mutex.
  A return value of 0 indicates success
*/
TSRM_API int tsrm_mutex_lock(MUTEX_T mutexp)
{
	return pthread_mutex_lock(mutexp);
}

/*
  Unlock a mutex.
  A return value of 0 indicates success
*/
TSRM_API int tsrm_mutex_unlock(MUTEX_T mutexp)
{
	return pthread_mutex_unlock(mutexp);
}
