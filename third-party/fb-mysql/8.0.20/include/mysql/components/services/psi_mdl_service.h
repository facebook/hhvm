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

#ifndef COMPONENTS_SERVICES_PSI_MDL_SERVICE_H
#define COMPONENTS_SERVICES_PSI_MDL_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/psi_mdl_bits.h>

BEGIN_SERVICE_DEFINITION(psi_mdl_v1)
create_metadata_lock_v1_t create_metadata_lock;
set_metadata_lock_status_v1_t set_metadata_lock_status;
destroy_metadata_lock_v1_t destroy_metadata_lock;
start_metadata_wait_v1_t start_metadata_wait;
end_metadata_wait_v1_t end_metadata_wait;
END_SERVICE_DEFINITION(psi_mdl_v1)

#endif /* COMPONENTS_SERVICES_PSI_MDL_SERVICE_H */
