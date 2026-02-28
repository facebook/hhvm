/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/base/composite_options_provider.h"

#include <stdarg.h>
#include <stddef.h>
#include <vector>

#include "client/base/i_options_provider.h"
#include "my_dbug.h"

using std::vector;
using namespace Mysql::Tools::Base::Options;

void Composite_options_provider::add_providers(I_options_provider *first, ...) {
  DBUG_ASSERT(this->m_options_providers.size() == 0);

  va_list options_to_add;
  va_start(options_to_add, first);

  this->add_provider(first);

  for (;;) {
    Options::I_options_provider *options_provider =
        va_arg(options_to_add, I_options_provider *);
    if (options_provider == nullptr) {
      break;
    }

    this->add_provider(options_provider);
  }
  va_end(options_to_add);
}

void Composite_options_provider::add_provider(
    I_options_provider *options_provider) {
  this->m_options_providers.push_back(options_provider);
  options_provider->set_option_changed_listener(this);
}

void Composite_options_provider::options_parsed() {
  vector<I_options_provider *>::iterator it;
  for (it = this->m_options_providers.begin();
       it != this->m_options_providers.end(); it++) {
    (*it)->options_parsed();
  }
}

vector<my_option> Composite_options_provider::generate_options() {
  vector<my_option> result;

  // Add options for all children providers.
  vector<I_options_provider *>::iterator it;
  for (it = this->m_options_providers.begin();
       it != this->m_options_providers.end(); it++) {
    vector<my_option> new_options = (*it)->generate_options();
    result.insert(result.end(), new_options.begin(), new_options.end());
  }

  // Add options from this provider.
  vector<my_option> new_options = Abstract_options_provider::generate_options();
  result.insert(result.end(), new_options.begin(), new_options.end());

  return result;
}
