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

#ifndef I_OPTIONS_PROVIDER_INCLUDED
#define I_OPTIONS_PROVIDER_INCLUDED

#include <vector>

#include "client/base/i_option_changed_listener.h"
#include "my_getopt.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Interface for basic options providers functionality.
 */
class I_options_provider : public I_option_changed_listener {
 public:
  /**
    Creates list of options provided by this provider.
    @returns list of my_getopt internal option data structures.
   */
  virtual std::vector<my_option> generate_options() = 0;
  /**
    Callback to be called when command-line options parsing have finished.
   */
  virtual void options_parsed() = 0;
  /**
    Sets optional option changes listener to which all changes in all options
    contained in this provider should be reported. This is used when this
    provider is attached to another.
   */
  virtual void set_option_changed_listener(
      I_option_changed_listener *listener) = 0;
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
