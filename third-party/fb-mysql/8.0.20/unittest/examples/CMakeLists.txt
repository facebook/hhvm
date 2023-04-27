# Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.
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

# Warnings about missing PGO profile data are not useful for unit tests.
DISABLE_MISSING_PROFILE_WARNING()

# We omit core-t here, since it will always fail.
SET(TESTS simple skip todo skip_all no_plan)
FOREACH(test ${TESTS})
  MYSQL_ADD_EXECUTABLE(${test}-t ${test}-t.cc ADD_TEST ${test})
  TARGET_LINK_LIBRARIES(${test}-t mytap)
ENDFOREACH()

# Simple executable which links only with libmytime. Not intended to
# be *used* for anything (hence SKIP_INSTALL), but still serves two
# purposes:
# 1: Can be used as a "hello world" example for applications
#    wishing to link with libmytime
# 2. Verifies that the library is actually independent (on platforms
#    where even unused symbols must be defined)
MYSQL_ADD_EXECUTABLE(mytime_client-t mytime_client-t.cc SKIP_INSTALL)
TARGET_LINK_LIBRARIES(mytime_client-t mytime ${LIBRT})
