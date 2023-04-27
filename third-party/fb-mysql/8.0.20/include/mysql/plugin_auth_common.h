#ifndef MYSQL_PLUGIN_AUTH_COMMON_INCLUDED
/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/mysql/plugin_auth_common.h

  This file defines constants and data structures that are the same for
  both client- and server-side authentication plugins.
*/
#define MYSQL_PLUGIN_AUTH_COMMON_INCLUDED

/** the max allowed length for a user name */
#define MYSQL_USERNAME_LENGTH 240

/**
  return values of the plugin authenticate_user() method.
*/

/**
  Authentication failed, plugin internal error.
  An error occurred in the authentication plugin itself.
  These errors are reported in table performance_schema.host_cache,
  column COUNT_AUTH_PLUGIN_ERRORS.
*/
#define CR_AUTH_PLUGIN_ERROR 3
/**
  Authentication failed, client server handshake.
  An error occurred during the client server handshake.
  These errors are reported in table performance_schema.host_cache,
  column COUNT_HANDSHAKE_ERRORS.
*/
#define CR_AUTH_HANDSHAKE 2
/**
  Authentication failed, user credentials.
  For example, wrong passwords.
  These errors are reported in table performance_schema.host_cache,
  column COUNT_AUTHENTICATION_ERRORS.
*/
#define CR_AUTH_USER_CREDENTIALS 1
/**
  Authentication failed. Additionally, all other CR_xxx values
  (libmysql error code) can be used too.

  The client plugin may set the error code and the error message directly
  in the MYSQL structure and return CR_ERROR. If a CR_xxx specific error
  code was returned, an error message in the MYSQL structure will be
  overwritten. If CR_ERROR is returned without setting the error in MYSQL,
  CR_UNKNOWN_ERROR will be user.
*/
#define CR_ERROR 0
/**
  Authentication (client part) was successful. It does not mean that the
  authentication as a whole was successful, usually it only means
  that the client was able to send the user name and the password to the
  server. If CR_OK is returned, the libmysql reads the next packet expecting
  it to be one of OK, ERROR, or CHANGE_PLUGIN packets.
*/
#define CR_OK -1
/**
  Authentication was successful.
  It means that the client has done its part successfully and also that
  a plugin has read the last packet (one of OK, ERROR, CHANGE_PLUGIN).
  In this case, libmysql will not read a packet from the server,
  but it will use the data at mysql->net.read_pos.

  A plugin may return this value if the number of roundtrips in the
  authentication protocol is not known in advance, and the client plugin
  needs to read one packet more to determine if the authentication is finished
  or not.
*/
#define CR_OK_HANDSHAKE_COMPLETE -2

/**
Flag to be passed back to server from authentication plugins via
authenticated_as when proxy mapping should be done by the server.
*/
#define PROXY_FLAG 0

/*
  We need HANDLE definition if on Windows. Define WIN32_LEAN_AND_MEAN (if
  not already done) to minimize amount of imported declarations.
*/
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

struct MYSQL_PLUGIN_VIO_INFO {
  enum {
    MYSQL_VIO_INVALID,
    MYSQL_VIO_TCP,
    MYSQL_VIO_SOCKET,
    MYSQL_VIO_PIPE,
    MYSQL_VIO_MEMORY
  } protocol;
  int socket; /**< it's set, if the protocol is SOCKET or TCP */
#ifdef _WIN32
  HANDLE handle; /**< it's set, if the protocol is PIPE or MEMORY */
#endif
};

/* state of an asynchronous operation */
enum net_async_status {
  NET_ASYNC_COMPLETE = 0,
  NET_ASYNC_NOT_READY,
  NET_ASYNC_ERROR,
  NET_ASYNC_COMPLETE_NO_MORE_RESULTS
};

/**
  Provides plugin access to communication channel
*/
typedef struct MYSQL_PLUGIN_VIO {
  /**
    Plugin provides a pointer reference and this function sets it to the
    contents of any incoming packet. Returns the packet length, or -1 if
    the plugin should terminate.
  */
  int (*read_packet)(struct MYSQL_PLUGIN_VIO *vio, unsigned char **buf);

  /**
    Plugin provides a buffer with data and the length and this
    function sends it as a packet. Returns 0 on success, 1 on failure.
  */
  int (*write_packet)(struct MYSQL_PLUGIN_VIO *vio, const unsigned char *packet,
                      int packet_len);

  /**
    Fills in a MYSQL_PLUGIN_VIO_INFO structure, providing the information
    about the connection.
  */
  void (*info)(struct MYSQL_PLUGIN_VIO *vio,
               struct MYSQL_PLUGIN_VIO_INFO *info);

  /**
    Non blocking version of read_packet. This function points buf to starting
    position of incoming packet. When this function returns NET_ASYNC_NOT_READY
    plugin should call this function again until all incoming packets are read.
    If return code is NET_ASYNC_COMPLETE, plugin can do further processing of
    read packets.
  */
  enum net_async_status (*read_packet_nonblocking)(struct MYSQL_PLUGIN_VIO *vio,
                                                   unsigned char **buf,
                                                   int *result);
  /**
    Non blocking version of write_packet. Sends data available in pkt of length
    pkt_len to server in asynchrnous way.
  */
  enum net_async_status (*write_packet_nonblocking)(
      struct MYSQL_PLUGIN_VIO *vio, const unsigned char *pkt, int pkt_len,
      int *result);

} MYSQL_PLUGIN_VIO;

#endif
