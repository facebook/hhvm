/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef SQL_NET_NS_H_
#define SQL_NET_NS_H_

#include "my_config.h"

#ifdef HAVE_SETNS
#include <string>

/**
  Set active network namespace specified by a name.

  @param network_namespace  the name of a network namespace to be set active

  @return false on success, true on error
  @note all opened descriptors used during function run are closed on error
*/
bool set_network_namespace(const std::string &network_namespace);

/**
  Restore original network namespace used to be active before a new network
  namespace has been set.

  @return false on success, true on failure
*/
bool restore_original_network_namespace();

/**
  Close file descriptors for every opened network namespace file
  and release a memory used by internal cache of opened network namespaces.
*/
void release_network_namespace_resources();

#endif /* HAVE_SETNS */

#endif /* SQL_NET_NS_H_ */
