/*
   Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef NAMED_PIPE_CONNECTION_INCLUDED
#define NAMED_PIPE_CONNECTION_INCLUDED

#include <Windows.h>
#include <string>

class Channel_info;
class THD;

/**
  This class abstracts Named pipe listener that setups a named pipe
  handle to listen and receive client connections.
*/
class Named_pipe_listener {
  std::string m_pipe_name;
  SECURITY_ATTRIBUTES *mp_sa_pipe_security;
  HANDLE m_pipe_handle;
  char m_pipe_path_name[512];
  HANDLE h_connected_pipe;
  OVERLAPPED m_connect_overlapped;

 public:
  /**
    Constructor for named pipe listener

    @param  pipe_name name for pipe used in CreateNamedPipe function.
  */
  Named_pipe_listener(const std::string *pipe_name)
      : m_pipe_name(*pipe_name),
        m_pipe_handle(INVALID_HANDLE_VALUE),
        mp_sa_pipe_security(nullptr) {}

  /**
    Set up a listener.

    @retval false listener listener has been setup successfully to listen for
    connect events true  failure in setting up the listener.
  */
  bool setup_listener();

  /**
    The body of the event loop that listen for connection events from clients.

    @retval Channel_info   Channel_info object abstracting the connected client
                           details for processing this connection.
  */
  Channel_info *listen_for_connection_event();

  /**
  Set the Windows group name whose users have full access to new instances of
  the named pipe

  @retval false access set successfully.
  true failed to change access.
  */
  bool update_named_pipe_full_access_group(const char *new_group_name);

  /**
    Close the listener
  */
  void close_listener();
};

#endif  // NAMED_PIPE_CONNECTION_INCLUDED.
