/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mc_fbtrace_info.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

static mc_fbtrace_t* mc_fbtrace_incref(mc_fbtrace_t* fbt) {
  assert(fbt);
  int newrefcount = __sync_add_and_fetch(&fbt->_refcount, 1);
  (void)newrefcount;
  assert(newrefcount > 0);
  return fbt;
}

static void mc_fbtrace_decref(mc_fbtrace_t* fbt) {
  assert(fbt);
  int newrefcount = __sync_add_and_fetch(&fbt->_refcount, -1);
  assert(newrefcount >= 0);
  if (newrefcount == 0) {
    free(fbt);
  }
}

static mc_fbtrace_t* new_mc_fbtrace() {
  mc_fbtrace_t* fbt = calloc(1, sizeof(*fbt));
  if (fbt == NULL) {
    return fbt;
  }
  mc_fbtrace_incref(fbt);
  return fbt;
}

mc_fbtrace_info_t* new_mc_fbtrace_info(int is_copy) {
  mc_fbtrace_info_t* fbt_i = calloc(1, sizeof(*fbt_i));
  if (fbt_i == NULL) {
    return fbt_i;
  }
  fbt_i->_refcount = 1;
  if (!is_copy) {
    fbt_i->fbtrace = new_mc_fbtrace();
    if (fbt_i->fbtrace == NULL) {
      free(fbt_i);
      return NULL;
    }
  }
  return fbt_i;
}

mc_fbtrace_info_t* mc_fbtrace_info_deep_copy(const mc_fbtrace_info_t* orig) {
  mc_fbtrace_info_t* new_copy = new_mc_fbtrace_info(1);
  memcpy(new_copy, orig, sizeof(mc_fbtrace_info_t));
  if (orig->fbtrace) {
    mc_fbtrace_incref(orig->fbtrace);
  }
  return new_copy;
}

void mc_fbtrace_info_decref(mc_fbtrace_info_t* fbt_i) {
  if (!fbt_i) {
    return;
  }
  int newrefcount = __sync_add_and_fetch(&fbt_i->_refcount, -1);
  assert(newrefcount >= 0);
  if (newrefcount == 0) {
    if (fbt_i->fbtrace) {
      mc_fbtrace_decref(fbt_i->fbtrace);
    }
    free(fbt_i);
  }
}

mc_fbtrace_info_t* mc_fbtrace_info_incref(mc_fbtrace_info_t* fbt_i) {
  if (!fbt_i) {
    return NULL;
  }
  int newrefcount = __sync_add_and_fetch(&fbt_i->_refcount, 1);
  (void)newrefcount;
  assert(newrefcount > 0);
  return fbt_i;
}
