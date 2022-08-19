/* Copyright (c) 2009, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_THREAD_INCLUDED
#define SQL_THREAD_INCLUDED

#include <mysql/psi/mysql_cond.h>
#include <mysql/psi/mysql_thread.h>

#include "my_thread.h"
#include "mysql/psi/mysql_mutex.h"

namespace thread {

/*
  An abstract class for creating/running/joining threads.
  Thread::start() will create a new pthread, and execute the run() function.
*/
class Thread {
 public:
  Thread() {}
  virtual ~Thread() {}

  /*
    Will create a new pthread, and invoke run();
    Returns the value from my_thread_create().
  */
  int start();

  /*
    You may invoke this to wait for the thread to finish.
    You should probalbly join() a thread before deleting it.
  */
  void join();

  // The handle of the thread (valid only if it is actually running).
  my_thread_handle thread_handle() const { return m_thread_handle; }

  /*
    A wrapper for the run() function.
    Users should *not* call this function directly, they should rather
    invoke the start() function.
  */
  static void run_wrapper(Thread *);

 protected:
  /*
    Define this function in derived classes.
    Users should *not* call this function directly, they should rather
    invoke the start() function.
  */
  virtual void run() = 0;

 private:
  my_thread_handle m_thread_handle;

  Thread(const Thread &);         /* Not copyable. */
  void operator=(const Thread &); /* Not assignable. */
};

// A barrier which can be used for one-time synchronization between threads.
class Notification {
 public:
  Notification();
  ~Notification();

  bool has_been_notified();
  void wait_for_notification();
  void notify();

 private:
  bool m_notified;
  mysql_cond_t m_cond;
  mysql_mutex_t m_mutex;

  Notification(const Notification &);   /* Not copyable. */
  void operator=(const Notification &); /* Not assignable. */
};

}  // namespace thread

#endif  // SQL_THREAD_INCLUDED
