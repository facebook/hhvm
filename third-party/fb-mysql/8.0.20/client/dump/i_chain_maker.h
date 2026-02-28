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

#ifndef I_CHAIN_MAKER_INCLUDED
#define I_CHAIN_MAKER_INCLUDED

#include "client/dump/chain_data.h"
#include "client/dump/i_chain_element.h"
#include "client/dump/i_dump_task.h"
#include "client/dump/i_object_reader.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class I_chain_maker : public virtual I_chain_element {
 public:
  /**
    Creates new chain for specified dump task. May return null if do not want
    to process specified object.
   */
  virtual I_object_reader *create_chain(Chain_data *chain_data,
                                        I_dump_task *dump_task) = 0;
  /**
    Frees resources used by specified chain.
   */
  virtual void delete_chain(uint64 chain_id, I_object_reader *chain) = 0;

  virtual void stop_queues() = 0;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
