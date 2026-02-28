/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <example_services.h>
#include <mysql/components/component_implementation.h>
#include <mysql/components/service_implementation.h>
#include <stddef.h>

#include "mysql/components/services/registry.h"

REQUIRES_SERVICE_PLACEHOLDER(example_math);

BEGIN_COMPONENT_REQUIRES(self_required_test_component)
REQUIRES_SERVICE(example_math), END_COMPONENT_REQUIRES();

class example_math_imp {
 public:
  static DEFINE_BOOL_METHOD(calculate_gcd, (int, int, int *)) { return true; }
};

BEGIN_SERVICE_IMPLEMENTATION(self_required_test_component, example_math)
example_math_imp::calculate_gcd, END_SERVICE_IMPLEMENTATION();

BEGIN_COMPONENT_PROVIDES(self_required_test_component)
PROVIDES_SERVICE(self_required_test_component, example_math),
    END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_METADATA(self_required_test_component)
END_COMPONENT_METADATA();

DECLARE_COMPONENT(self_required_test_component,
                  "mysql:self_required_test_component")
nullptr, nullptr END_DECLARE_COMPONENT();

DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(self_required_test_component)
    END_DECLARE_LIBRARY_COMPONENTS
