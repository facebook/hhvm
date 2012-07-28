/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "gdb-jit.h"
#include <util/lock.h>
#include <runtime/base/execution_context.h>

using namespace HPHP;

struct jit_descriptor __jit_debug_descriptor = { 1, 0, 0, 0 };

void unregister_gdb_hook(struct jit_code_entry *e) {
  __jit_debug_descriptor.action_flag = JIT_UNREGISTER_FN;
  __jit_debug_descriptor.relevant_entry = e;
  __jit_debug_register_code();
}

void delete_symfile(const char *old) {
  struct jit_code_entry *e, *prev;

  e = __jit_debug_descriptor.first_entry;
  prev = NULL;
  while (e != NULL) {
    if (e->symfile_addr == old) {
      if (prev !=  NULL) {
        prev->next_entry = e->next_entry;
      } else {
        __jit_debug_descriptor.first_entry = e->next_entry;
      }
      if (e->next_entry != NULL) {
        e->next_entry->prev_entry = prev;
      }
      unregister_gdb_hook(e);
      free((void *)e->symfile_addr);
      free(e);
      return;
    }
    prev = e;
    e = e->next_entry;
  }
}

Mutex gdbLock;

void unregister_gdb_chunk(DwarfChunk* d) {
  Lock lock(gdbLock);

  if (d->m_symfile != NULL) {
    delete_symfile(d->m_symfile);
    d->m_symfile = NULL;
  }
}

int register_gdb_hook(char *symfile_addr, uint64_t symfile_size,
  DwarfChunk* d) {
  struct jit_code_entry *entry;
  Lock lock(gdbLock);

  if ((entry =
      (struct jit_code_entry *)malloc(sizeof (struct jit_code_entry))) == NULL)
    return -1;

  entry->symfile_addr = symfile_addr;
  entry->symfile_size = symfile_size;

  if (d->m_symfile != NULL) {
    delete_symfile(d->m_symfile);
  }
  d->m_symfile = symfile_addr;

  entry->prev_entry = NULL;
  entry->next_entry = __jit_debug_descriptor.first_entry;
  if (__jit_debug_descriptor.first_entry) {
    __jit_debug_descriptor.first_entry->prev_entry = entry;
  }
  __jit_debug_descriptor.first_entry = entry;
  __jit_debug_descriptor.relevant_entry = entry;

  __jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
  __jit_debug_register_code();
  return 0;
}
