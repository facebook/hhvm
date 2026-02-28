/* Copyright (c) 2012, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef NAMED_PIPE_INCLUDED
#define NAMED_PIPE_INCLUDED

#include <windows.h>

const DWORD NAMED_PIPE_OWNER_PERMISSIONS = GENERIC_READ | GENERIC_WRITE;
const DWORD NAMED_PIPE_EVERYONE_PERMISSIONS =
    FILE_READ_ATTRIBUTES | FILE_READ_DATA | FILE_WRITE_ATTRIBUTES |
    FILE_WRITE_DATA | SYNCHRONIZE | READ_CONTROL;
const DWORD NAMED_PIPE_FULL_ACCESS_GROUP_PERMISSIONS =
    GENERIC_READ | GENERIC_WRITE;

HANDLE create_server_named_pipe(SECURITY_ATTRIBUTES **ppsec_attr,
                                DWORD buffer_size, const char *name,
                                char *name_buf, size_t buflen,
                                const char *full_access_group_name = nullptr);

bool is_valid_named_pipe_full_access_group(const char *group_name);
bool my_security_attr_add_rights_to_group(SECURITY_ATTRIBUTES *psa,
                                          const char *group_name,
                                          DWORD group_rights);
#define DEFAULT_NAMED_PIPE_FULL_ACCESS_GROUP "*everyone*"
#endif /* NAMED_PIPE_INCLUDED */
