/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_FILE_SERVICE_H
#define COMPONENTS_SERVICES_PSI_FILE_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/psi_file_bits.h>

/*
  Version 1.
  Introduced in MySQL 8.0.3
  Abandoned in MySQL 8.0.19
  Status: Removed, use version 2 instead.
*/

/*
  Version 2.
  Introduced in MySQL 8.0.19
  Status: active
*/

BEGIN_SERVICE_DEFINITION(psi_file_v2)
/** @sa register_file_v1_t. */
register_file_v1_t register_file;
/** @sa create_file_v1_t. */
create_file_v1_t create_file;
/** @sa get_thread_file_name_locker_v1_t. */
get_thread_file_name_locker_v1_t get_thread_file_name_locker;
/** @sa get_thread_file_stream_locker_v1_t. */
get_thread_file_stream_locker_v1_t get_thread_file_stream_locker;
/** @sa get_thread_file_descriptor_locker_v1_t. */
get_thread_file_descriptor_locker_v1_t get_thread_file_descriptor_locker;
/** @sa start_file_open_wait_v1_t. */
start_file_open_wait_v1_t start_file_open_wait;
/** @sa end_file_open_wait_v1_t. */
end_file_open_wait_v1_t end_file_open_wait;
/** @sa end_file_open_wait_and_bind_to_descriptor_v1_t. */
end_file_open_wait_and_bind_to_descriptor_v1_t
    end_file_open_wait_and_bind_to_descriptor;
/** @sa end_temp_file_open_wait_and_bind_to_descriptor_v1_t. */
end_temp_file_open_wait_and_bind_to_descriptor_v1_t
    end_temp_file_open_wait_and_bind_to_descriptor;
/** @sa start_file_wait_v1_t. */
start_file_wait_v1_t start_file_wait;
/** @sa end_file_wait_v1_t. */
end_file_wait_v1_t end_file_wait;
/** @sa start_file_close_wait_v1_t. */
start_file_close_wait_v1_t start_file_close_wait;
/** @sa end_file_close_wait_v1_t. */
end_file_close_wait_v1_t end_file_close_wait;
/** @sa start_file_rename_wait_v1_t. */
start_file_rename_wait_v1_t start_file_rename_wait;
/** @sa rename_file_close_wait_v1_t. */
end_file_rename_wait_v1_t end_file_rename_wait;
END_SERVICE_DEFINITION(psi_file_v2)

#endif /* COMPONENTS_SERVICES_PSI_FILE_SERVICE_H */
