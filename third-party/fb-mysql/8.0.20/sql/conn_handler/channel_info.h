/*
   Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_CHANNEL_INFO_INCLUDED
#define SQL_CHANNEL_INFO_INCLUDED

#include <sys/types.h>

#include "my_inttypes.h"
#include "my_systime.h"  // my_micro_time
#include "violite.h"

class THD;

typedef Vio Vio;

/**
  This abstract base class represents connection channel information
  about a new connection. Its subclasses encapsulate differences
  between different connection channel types.

  Currently we support local and TCP/IP sockets (all platforms),
  named pipes and shared memory (Windows only).
*/
class Channel_info {
  ulonglong prior_thr_create_utime;

 protected:
  /**
    Create and initialize a Vio object.

    @returns a pointer to the initialized a vio object.
  */
  virtual Vio *create_and_init_vio() const = 0;

  Channel_info() : prior_thr_create_utime(0) {}

 public:
  virtual ~Channel_info() {}

  /**
    Instantiate and initialize THD object and vio.

    @returns pointer to initialized THD object.
    @retval NULL THD object allocation fails.
  */
  virtual THD *create_thd();

  /**
    Send error back to the client and close the channel.

    @param errorcode   code indicating type of error.
    @param error       operating system specific error code.
    @param senderror   true if the error need to be sent to
                       client else false.
  */
  virtual void send_error_and_close_channel(uint errorcode, int error,
                                            bool senderror);

  ulonglong get_prior_thr_create_utime() const {
    return prior_thr_create_utime;
  }

  void set_prior_thr_create_utime() {
    prior_thr_create_utime = my_micro_time();
  }

  virtual bool is_admin_connection() const { return false; }
};

#endif  // SQL_CHANNEL_INFO_INCLUDED.
