/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_ERROR_BITS_H
#define COMPONENTS_SERVICES_PSI_ERROR_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_error Error Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

enum PSI_error_operation {
  PSI_ERROR_OPERATION_RAISED = 0,
  PSI_ERROR_OPERATION_HANDLED
};
typedef enum PSI_error_operation PSI_error_operation;

/**
  Log the error seen in Performance Schema buffers.
  @param error_num MySQL error number
  @param error_operation operation on error (PSI_ERROR_OPERATION_*)
*/
typedef void (*log_error_v1_t)(unsigned int error_num,
                               PSI_error_operation error_operation);

/** @} (end of group psi_abi_error) */

#endif /* COMPONENTS_SERVICES_PSI_ERROR_BITS_H */
