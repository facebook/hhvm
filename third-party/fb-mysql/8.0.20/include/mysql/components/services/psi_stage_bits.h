/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_STAGE_BITS_H
#define COMPONENTS_SERVICES_PSI_STAGE_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_stage Stage Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented stage key.
  To instrument a stage, a stage key must be obtained using @c register_stage.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_stage_key;

/**
  @def PSI_STAGE_VERSION_1
  Performance Schema Stage Interface number for version 1.
  This version is supported.
*/
#define PSI_STAGE_VERSION_1 1

/**
  @def PSI_CURRENT_STAGE_VERSION
  Performance Schema Stage Interface number for the most recent version.
  The most current version is @c PSI_STAGE_VERSION_1
*/
#define PSI_CURRENT_STAGE_VERSION 1

/**
  Interface for an instrumented stage progress.
  This is a public structure, for efficiency.
*/
struct PSI_stage_progress_v1 {
  unsigned long long m_work_completed;
  unsigned long long m_work_estimated;
};
typedef struct PSI_stage_progress_v1 PSI_stage_progress_v1;

/**
  Stage instrument information.
  @since PSI_STAGE_VERSION_1
  This structure is used to register an instrumented stage.
*/
struct PSI_stage_info_v1 {
  /** The registered stage key. */
  PSI_stage_key m_key{0};
  /** The name of the stage instrument to register. */
  const char *m_name{nullptr};
  /**
    The flags of the stage instrument to register.
    @sa PSI_FLAG_PROGRESS
  */
  unsigned int m_flags{0};
  /** Documentation. */
  const char *m_documentation{nullptr};
};
typedef struct PSI_stage_info_v1 PSI_stage_info_v1;

/**
  Stage registration API.
  @param category a category name
  @param info an array of stage info to register
  @param count the size of the info array
*/
typedef void (*register_stage_v1_t)(const char *category,
                                    struct PSI_stage_info_v1 **info, int count);

/**
  Start a new stage, and implicitly end the previous stage.
  @param key the key of the new stage
  @param src_file the source file name
  @param src_line the source line number
  @return the new stage progress
*/
typedef PSI_stage_progress_v1 *(*start_stage_v1_t)(PSI_stage_key key,
                                                   const char *src_file,
                                                   int src_line);

/**
  Get the current stage progress.
  @return the stage progress
*/
typedef PSI_stage_progress_v1 *(*get_current_stage_progress_v1_t)(void);

/** End the current stage. */
typedef void (*end_stage_v1_t)(void);

typedef struct PSI_stage_info_v1 PSI_stage_info;
typedef struct PSI_stage_progress_v1 PSI_stage_progress;

/** @} (end of group psi_abi_stage) */

#endif /* COMPONENTS_SERVICES_PSI_STAGE_BITS_H */
