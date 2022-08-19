/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPOSITE_OPTIONS_PROVIDER_INCLUDED
#define COMPOSITE_OPTIONS_PROVIDER_INCLUDED

#include "client/base/abstract_options_provider.h"
#include "client/base/i_option.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Provider that aggregates options from other providers.
  It is still a provider, so can have have own options added.
 */
class Composite_options_provider : public Abstract_options_provider {
 public:
  /**
    Adds new providers to list. Last parameter value must be NULL.
   */
  void add_providers(I_options_provider *first, /*I_options_provider* */...);
  /**
    Adds new provider to list.
   */
  void add_provider(I_options_provider *options_provider);

  /**
    This callback is to be called after all options were parsed.
   */
  virtual void options_parsed();

  /**
    Aggregates all options from itsself and all contained providers.
   */
  virtual std::vector<my_option> generate_options();

 private:
  std::vector<I_options_provider *> m_options_providers;
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
