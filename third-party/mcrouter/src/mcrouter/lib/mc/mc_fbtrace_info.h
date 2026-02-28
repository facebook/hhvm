/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef FB_MEMCACHE_MC_FBTRACE_H
#define FB_MEMCACHE_MC_FBTRACE_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "fbtrace/libfbtrace/c/fbtrace.h"
#include "fbtrace/libfbtrace/c/fbtrace_encode.h"

__BEGIN_DECLS

typedef struct mc_fbtrace_s {
  int _refcount;
  fbtrace_node_t node;
} mc_fbtrace_t;

typedef struct mc_fbtrace_info_s {
  int _refcount;
  mc_fbtrace_t* fbtrace;
  fbtrace_remote_node_t child_node;
  char metadata[FBTRACE_METADATA_SZ + 1];
} mc_fbtrace_info_t;

mc_fbtrace_info_t* new_mc_fbtrace_info(int is_copy);
mc_fbtrace_info_t* mc_fbtrace_info_deep_copy(const mc_fbtrace_info_t* orig);
void mc_fbtrace_info_decref(mc_fbtrace_info_t* fbt_w);
mc_fbtrace_info_t* mc_fbtrace_info_incref(mc_fbtrace_info_t* fbt_w);

__END_DECLS

#endif // FB_MEMCACHE_MC_FBTRACE_H
