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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "sql/mdl.h"
#include "unittest/gunit/thread_utils.h"

using thread::Notification;
using thread::Thread;

namespace {

const int counter_start_value = 42;

class NotificationThread : public Thread {
 public:
  NotificationThread(Notification *start_notification,
                     Notification *end_notfication, int *counter)
      : m_start_notification(start_notification),
        m_end_notification(end_notfication),
        m_counter(counter) {}

  virtual void run() {
    // Verify counter, increment it, notify the main thread.
    EXPECT_EQ(counter_start_value, *m_counter);
    (*m_counter) += 1;
    m_start_notification->notify();

    // Wait for notification from other thread.
    m_end_notification->wait_for_notification();
    EXPECT_EQ(counter_start_value, *m_counter);

    // Set counter again before returning from thread.
    (*m_counter) += 1;
  }

 private:
  Notification *m_start_notification;
  Notification *m_end_notification;
  int *m_counter;

  NotificationThread(const NotificationThread &);  // Not copyable.
  void operator=(const NotificationThread &);      // Not assignable.
};

/*
  A basic, single-threaded test of Notification.
 */
TEST(Notification, Notify) {
  Notification notification;
  EXPECT_FALSE(notification.has_been_notified());
  notification.notify();
  EXPECT_TRUE(notification.has_been_notified());
}

/*
  Starts a thread, and verifies that the notification/synchronization
  mechanism works.
 */
TEST(NotificationThread, StartAndWait) {
  Notification start_notification;
  Notification end_notfication;
  int counter = counter_start_value;
  NotificationThread notification_thread(&start_notification, &end_notfication,
                                         &counter);
  notification_thread.start();

  // Wait for the other thread to increment counter, and notify us.
  start_notification.wait_for_notification();
  EXPECT_EQ(counter_start_value + 1, counter);
  EXPECT_TRUE(start_notification.has_been_notified());

  // Reset counter, and notify other thread.
  counter = counter_start_value;
  end_notfication.notify();
  notification_thread.join();

  // We should see the final results of the thread we have joined.
  EXPECT_EQ(counter_start_value + 1, counter);
}

}  // namespace
