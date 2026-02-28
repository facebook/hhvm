/* Copyright (c) 2012, 2020, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MYSQL_MDL_H
#define MYSQL_MDL_H

/**
  @file include/mysql/psi/mysql_mdl.h
  Instrumentation helpers for metadata locks.
*/

#include "mysql/psi/psi_mdl.h"

#ifndef PSI_METADATA_CALL
#define PSI_METADATA_CALL(M) psi_mdl_service->M
#endif

/**
  @defgroup psi_api_mdl Metadata Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_mdl_create(K, M, A)
  Instrumented metadata lock creation.
  @param I Metadata lock identity
  @param K Metadata key
  @param T Metadata lock type
  @param D Metadata lock duration
  @param S Metadata lock status
  @param F request source file
  @param L request source line
*/

#ifdef HAVE_PSI_METADATA_INTERFACE
#define mysql_mdl_create(I, K, T, D, S, F, L) \
  inline_mysql_mdl_create(I, K, T, D, S, F, L)
#else
#define mysql_mdl_create(I, K, T, D, S, F, L) NULL
#endif

#ifdef HAVE_PSI_METADATA_INTERFACE
#define mysql_mdl_set_status(L, S) inline_mysql_mdl_set_status(L, S)
#else
#define mysql_mdl_set_status(L, S) \
  do {                             \
  } while (0)
#endif

/**
  @def mysql_mdl_destroy(M)
  Instrumented metadata lock destruction.
  @param M Metadata lock
*/
#ifdef HAVE_PSI_METADATA_INTERFACE
#define mysql_mdl_destroy(M) inline_mysql_mdl_destroy(M, __FILE__, __LINE__)
#else
#define mysql_mdl_destroy(M) \
  do {                       \
  } while (0)
#endif

#ifdef HAVE_PSI_METADATA_INTERFACE

static inline PSI_metadata_lock *inline_mysql_mdl_create(
    void *identity, const MDL_key *mdl_key, enum_mdl_type mdl_type,
    enum_mdl_duration mdl_duration, MDL_ticket::enum_psi_status mdl_status,
    const char *src_file, uint src_line) {
  PSI_metadata_lock *result;

  /* static_cast: Fit a round C++ enum peg into a square C int hole ... */
  result = PSI_METADATA_CALL(create_metadata_lock)(
      identity, mdl_key, static_cast<opaque_mdl_type>(mdl_type),
      static_cast<opaque_mdl_duration>(mdl_duration),
      static_cast<opaque_mdl_status>(mdl_status), src_file, src_line);

  return result;
}

static inline void inline_mysql_mdl_set_status(
    PSI_metadata_lock *psi, MDL_ticket::enum_psi_status mdl_status) {
  if (psi != nullptr) {
    PSI_METADATA_CALL(set_metadata_lock_status)(psi, mdl_status);
  }
}

static inline void inline_mysql_mdl_destroy(PSI_metadata_lock *psi,
                                            const char *, uint) {
  if (psi != nullptr) {
    PSI_METADATA_CALL(destroy_metadata_lock)(psi);
  }
}
#endif /* HAVE_PSI_METADATA_INTERFACE */

/** @} (end of group psi_api_mdl) */

#endif
