#ifndef CONNECTION_HANDLER_INCLUDED
#define CONNECTION_HANDLER_INCLUDED

/* Copyright (c) 2007, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_inttypes.h"

class THD;
class Channel_info;
class Connection_handler_manager;

/**
  This abstract base class represents how connections are processed,
  most importantly how they map to OS threads.
*/
class Connection_handler {
 protected:
  friend class Connection_handler_manager;

  Connection_handler() {}
  virtual ~Connection_handler() {}

  /**
    Add a connection.

    @param channel_info     Pointer to the Channel_info object.

    @note If this function is successful (returns false), the ownership of
    channel_info is transferred. Otherwise the caller is responsible for
    its destruction.

    @return true if processing of the new connection failed, false otherwise.
  */
  virtual bool add_connection(Channel_info *channel_info) = 0;

  /**
    @return Maximum number of threads that can be created
            by this connection handler.
  */
  virtual uint get_max_threads() const = 0;
};

#endif  // CONNECTION_HANDLER_INCLUDED
