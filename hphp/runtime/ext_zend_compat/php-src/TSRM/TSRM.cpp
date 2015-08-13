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

namespace HPHP {
  static std::vector<tsrm_resource_type> resource_types_table;
  IMPLEMENT_THREAD_LOCAL(TSRMStorageVector, tsrm_thread_resources);
}

#define TSRM_ERROR(args)

TSRM_API ts_rsrc_id
ts_allocate_id(ts_rsrc_id *rsrc_id, size_t size, ts_allocate_ctor ctor,
               ts_allocate_dtor dtor) {
  TSRM_ERROR((TSRM_ERROR_LEVEL_CORE,
             "Obtaining a new resource id, %d bytes", size));

  auto& table = HPHP::resource_types_table;
  /* obtain a resource id */
  table.resize(table.size() + 1);
  *rsrc_id = TSRM_SHUFFLE_RSRC_ID(table.size() - 1);
  TSRM_ERROR((TSRM_ERROR_LEVEL_CORE, "Obtained resource id %d", *rsrc_id));

  /* store the new resource type in the resource sizes table */
  auto& type = table[TSRM_UNSHUFFLE_RSRC_ID(*rsrc_id)];
  type.size = size;
  type.ctor = ctor;
  type.dtor = dtor;
  type.done = 0;

  TSRM_ERROR((TSRM_ERROR_LEVEL_CORE,
             "Successfully allocated new resource id %d", *rsrc_id));
  return *rsrc_id;
}

namespace HPHP {

void* ts_init_resource(int id) {
  assert(id != 0);
  TSRMLS_FETCH();
  HPHP::TSRMStorageVector& vec = *HPHP::tsrm_thread_resources;
  auto index = TSRM_UNSHUFFLE_RSRC_ID(id);
  if (index >= vec.size()) {
    vec.resize(index + 1);
  }
  tsrm_resource_type& type = HPHP::resource_types_table.at(index);
  if (!vec[index]) {
    // TODO: t7925981 could we use req::malloc?
    vec[index] = malloc(type.size);
    if (type.ctor) {
      type.ctor(vec[index] TSRMLS_CC);
    }
  }
  return vec[index];
}

void ts_scan_resources(IMarker& mark) {
  HPHP::TSRMStorageVector& vec = *HPHP::tsrm_thread_resources;
  auto ntypes = resource_types_table.size();
  auto nres = vec.size();
  for (size_t i = 0, n = std::min(ntypes, nres); i < n; ++i) {
    if (!vec[i]) continue;
    mark(vec[i], resource_types_table[i].size);
    // maybe add scan() to tsrm_resource_type in addition to ctor/dtor
  }
}

} // HPHP

void ts_free_id(ts_rsrc_id id) {
  HPHP::TSRMStorageVector& vec = *HPHP::tsrm_thread_resources;
  int j = TSRM_UNSHUFFLE_RSRC_ID(id);
  assert(id != 0);
  TSRMLS_FETCH();
  if (j < vec.size() && vec[j] != nullptr) {
    if (HPHP::resource_types_table[j].dtor) {
      HPHP::resource_types_table[j].dtor(vec[j] TSRMLS_CC);
    }
    free(vec[j]);
    vec[j] = nullptr;
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
