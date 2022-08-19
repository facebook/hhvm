# Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

#
# Configuration for building Windows Authentication Plugin (client-side)
#

ADD_DEFINITIONS(-DSECURITY_WIN32)
ADD_DEFINITIONS(-DDEBUG_ERRROR_LOG)  # no error logging in production builds
ADD_DEFINITIONS(-DWINAUTH_USE_DBUG_LIB)      # it is OK to use dbug library in statically
                                             # linked plugin

SET(HEADERS common.h handshake.h)
SET(PLUGIN_SOURCES
  common.cc
  handshake.cc
  handshake_client.cc
  log_client.cc
  plugin_client.cc
  )

ADD_CONVENIENCE_LIBRARY(auth_win_client ${PLUGIN_SOURCES} ${HEADERS}
  LINK_LIBRARIES Secur32)

# In IDE, group headers in a separate folder.

SOURCE_GROUP(Headers REGULAR_EXPRESSION ".*h$")
