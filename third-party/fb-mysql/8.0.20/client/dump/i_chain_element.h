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

#ifndef I_CHAIN_ELEMENT_INCLUDED
#define I_CHAIN_ELEMENT_INCLUDED

#include "client/dump/i_progress_reporter.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class Item_processing_data;

/**
  Interface for all objects that can process data in any part of dump process.
 */
class I_chain_element : public virtual I_progress_reporter {
 public:
  virtual ~I_chain_element();

  /**
    Returns an application unique ID of this chain element object. This helps
    progress watching with multiple parts of chain during all objects
    processing.
   */
  virtual uint64 get_id() const = 0;

 protected:
  /**
    This callback can be requested to be called by child for any object
    processing. This will be called when the object processing has completed.
    Note that this function may be called from multiple threads so all
    implementations must be thread-safe.
   */
  virtual void item_completion_in_child_callback(
      Item_processing_data *item_processed) = 0;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
