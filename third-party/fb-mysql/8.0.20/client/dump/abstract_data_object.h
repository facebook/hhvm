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

#ifndef ABSTRACT_DATA_OBJECT_INCLUDED
#define ABSTRACT_DATA_OBJECT_INCLUDED

#include <string>

#include "client/dump/i_data_object.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Base class for all main DB objects.
 */
class Abstract_data_object : public I_data_object {
 public:
  virtual ~Abstract_data_object();

  /**
    Returns an unique ID of this DB object. This helps progress watching with
    multiple parts of chain during object processing (including queuing).
   */
  uint64 get_id() const;
  /**
    Returns schema in which object is contained.
   */
  std::string get_schema() const;
  /**
    Returns name of object in schema.
   */
  std::string get_name() const;

 protected:
  Abstract_data_object(uint64 id, const std::string &name,
                       const std::string &schema);

 private:
  /**
    An unique ID of this DB object. This helps progress watching with multiple
    parts of chain during object processing (including queuing).
   */
  uint64 m_id;
  /**
    Schema in which object is contained.
    */
  std::string m_schema;
  /**
    Name of object in schema.
   */
  std::string m_name;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
