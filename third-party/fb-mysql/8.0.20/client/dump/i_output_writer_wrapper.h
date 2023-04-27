/*
  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef I_OUTPUT_WRITER_WRAPPER_INCLUDED
#define I_OUTPUT_WRITER_WRAPPER_INCLUDED

#include "client/dump/i_output_writer.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Represents class that directs execution of dump tasks to Output Writers.
 */
class I_output_writer_wrapper {
 public:
  virtual ~I_output_writer_wrapper() {}
  /**
    Add new Output Writer to supply formatted strings to.
   */
  virtual void register_output_writer(I_output_writer *new_output_writer) = 0;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
