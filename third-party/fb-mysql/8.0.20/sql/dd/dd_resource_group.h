/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef DD_RESOURCE_GROUP_INCLUDED
#define DD_RESOURCE_GROUP_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type

class THD;
namespace resourcegroups {
class Resource_group;
}
namespace dd {
namespace cache {
class Dictionary_client;
}

/**
  Check if resource group exists in the data dictionary.

  @param          dd_client            Dictionary client.
  @param          resource_group_name  Name of the resource group.
  @param  [out]   exists               Value set to true if DD object found
                                        else false.
  @retval         true                 Failure (error has been reported).
  @retval         false                Success.
*/

bool resource_group_exists(dd::cache::Dictionary_client *dd_client,
                           const String_type &resource_group_name,
                           bool *exists);

/**
  Create a DD object and persist it to DD table resourcegroup.

  @param  thd          Thread handle.
  @param  res_grp_ref  Reference to resource group.

  @retval true  Resource group creation failed.
  @retval false Resource group creation succeeded.
*/

bool create_resource_group(THD *thd,
                           const resourcegroups::Resource_group &res_grp_ref);

/**
  Update a resource group and persist it to DD table resourcegroup.

  @param thd                 Thread handle
  @param resource_grp_name   Name of the resource group.
  @param res_grp_ref         Reference to resource group.

  @retval true   Updating Resource group failed.
  @retval false  Updating Resource group succeeded.
*/

bool update_resource_group(THD *thd, const String_type &resource_grp_name,
                           const resourcegroups::Resource_group &res_grp_ref);

/**
  Drop a resource group from DD table resourcegroup.

  @param thd                 Thread handle.
  @param resource_grp_name   Name of resource group to be dropped.

  @retval true  if resource group drop failed.
  @retval false if resource group drop succeeded.
*/

bool drop_resource_group(THD *thd, const String_type resource_grp_name);
}  // namespace dd
#endif  // DD_RESOURCE_GROUP_INCLUDED
