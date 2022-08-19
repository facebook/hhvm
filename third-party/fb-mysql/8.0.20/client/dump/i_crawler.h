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

#ifndef I_CRAWLER_INCLUDED
#define I_CRAWLER_INCLUDED

#include "client/dump/i_chain_element.h"
#include "client/dump/i_chain_maker.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class I_crawler : public virtual I_chain_element {
 public:
  /**
    Enumerates all objects it can access, gets chains from all registered
    chain_maker for each object and then execute each chain.
   */
  virtual void enumerate_objects() = 0;
  /**
    Adds new Chain Maker to ask for chains for found objects.
   */
  virtual void register_chain_maker(I_chain_maker *new_chain_maker) = 0;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
