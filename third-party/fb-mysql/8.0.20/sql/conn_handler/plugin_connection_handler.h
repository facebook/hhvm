#ifndef PLUGIN_CONNECTION_HANDLER_INCLUDED
#define PLUGIN_CONNECTION_HANDLER_INCLUDED

/*
   Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "mysql/thread_pool_priv.h"  // Plugin_connection_handler_functions
#include "sql/conn_handler/connection_handler.h"  // Connection_handler

class THD;

/**
   This is a wrapper class around global free functions implemented
   by connection handler plugins (e.g. thread pool). So instead of
   plugins implementing a Connection_handler subclass, they supply
   a set of function pointers to my_connection_handler_set() which
   instantiates Plugin_connection_handler.

   @see Connection_handler_functions struct.
*/

class Plugin_connection_handler : public Connection_handler {
  Connection_handler_functions *m_functions;

  Plugin_connection_handler(const Plugin_connection_handler &);
  Plugin_connection_handler &operator=(const Plugin_connection_handler &);

 public:
  Plugin_connection_handler() : m_functions(nullptr) {}

  void set_functions(Connection_handler_functions *functions) {
    m_functions = functions;
  }

  // The function is mostly useless as it doesn't signal the end of usage of
  // functions. The listener thread could still be already in the process of
  // callinng add_connection(), having loaded the connection handler pointer
  // (so it cannot be released and m_functions cannot be reset) and having
  // loaded the functions pointer (so functions also cannot be released). So
  // realistically the end() callback could only do something at shutdown after
  // the listener stopped.
  void end_functions() {
    auto functions = m_functions;
    if (functions) {
      functions->end();
    }
  }

 protected:
  virtual bool add_connection(Channel_info *channel_info) {
    return m_functions->add_connection(channel_info);
  }

  virtual uint get_max_threads() const { return m_functions->max_threads; }
};

#endif  // PLUGIN_CONNECTION_HANDLER_INCLUDED
