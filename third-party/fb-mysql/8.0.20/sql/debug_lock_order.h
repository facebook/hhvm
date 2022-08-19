/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef DEBUG_LOCK_ORDER_H
#define DEBUG_LOCK_ORDER_H

#include "my_sys.h"

struct PSI_bootstrap;

struct LO_node_properties {
  const char *m_name;
  const char *m_operation;
  int m_flags;
};

struct LO_authorised_arc {
  char *m_from_name;
  char *m_from_state;
  char *m_to_name;
  bool m_op_recursive;
  char *m_to_operation;
  int m_flags;
  char *m_constraint;
  char *m_comment;
};

#define LO_FLAG_TRACE 1 << 0
#define LO_FLAG_DEBUG 1 << 1
#define LO_FLAG_LOOP 1 << 2
#define LO_FLAG_IGNORED 1 << 3
#define LO_FLAG_BIND 1 << 4
#define LO_FLAG_UNFAIR 1 << 5
/* Micro arcs, generated from macro declarations. */
#define LO_FLAG_MICRO 1 << 6

struct LO_global_param {
  bool m_enabled;
  char *m_out_dir;
  char *m_dependencies_1;
  char *m_dependencies_2;
  bool m_print_txt;
  bool m_trace_loop;
  bool m_debug_loop;
  bool m_trace_missing_arc;
  bool m_debug_missing_arc;
  bool m_trace_missing_unlock;
  bool m_debug_missing_unlock;
  bool m_trace_bad_unlock;
  bool m_debug_bad_unlock;
  bool m_trace_missing_key;
  bool m_debug_missing_key;
};

extern LO_global_param lo_param;

int LO_init(
    struct LO_global_param *param, PSI_thread_bootstrap **thread_bootstrap,
    PSI_mutex_bootstrap **mutex_bootstrap,
    PSI_rwlock_bootstrap **rwlock_bootstrap,
    PSI_cond_bootstrap **cond_bootstrap, PSI_file_bootstrap **file_bootstrap,
    PSI_socket_bootstrap **socket_bootstrap,
    PSI_table_bootstrap **table_bootstrap, PSI_mdl_bootstrap **mdl_bootstrap,
    PSI_idle_bootstrap **idle_bootstrap, PSI_stage_bootstrap **stage_bootstrap,
    PSI_statement_bootstrap **statement_bootstrap,
    PSI_transaction_bootstrap **transaction_bootstrap,
    PSI_memory_bootstrap **memory_bootstrap);

void LO_activate();

class LO_graph;
void LO_add_authorised_arc(LO_graph *g, const LO_authorised_arc *arc);
void LO_add_node_properties(LO_graph *g, const LO_node_properties *node);

void LO_dump();

void LO_cleanup();

/*
  Helper for the performance schema.
*/
PSI_thread *LO_get_chain_thread(PSI_thread *psi);

#endif
