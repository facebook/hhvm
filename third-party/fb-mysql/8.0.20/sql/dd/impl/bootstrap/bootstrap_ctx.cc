/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::DD_bootstrap_ctx
#include "sql/dd/impl/tables/dd_properties.h"     // dd::tables::DD_properties

namespace dd {
namespace bootstrap {

DD_bootstrap_ctx &DD_bootstrap_ctx::instance() {
  static DD_bootstrap_ctx s_instance;
  return s_instance;
}

bool DD_bootstrap_ctx::is_above_minor_downgrade_threshold(THD *thd) const {
  uint minor_downgrade_threshold = 0;
  bool exists = false;
  /*
    If we successfully get hold of the threshold, and it exists, and
    the target DD version is above or equal to the threshold, then we
    return true.
  */
  return (!dd::tables::DD_properties::instance().get(
              thd, "MINOR_DOWNGRADE_THRESHOLD", &minor_downgrade_threshold,
              &exists) &&
          exists && dd::DD_VERSION >= minor_downgrade_threshold);
}

}  // namespace bootstrap
}  // namespace dd
