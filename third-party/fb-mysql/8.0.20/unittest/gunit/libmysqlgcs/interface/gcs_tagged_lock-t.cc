/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "gcs_base_test.h"

#include "gcs_tagged_lock.h"

namespace gcs_tagged_lock_unittest {

class GcsTaggedLock : public GcsBaseTest {
 protected:
  GcsTaggedLock() {}
};

TEST_F(GcsTaggedLock, SuccessfulOptimisticRead) {
  Gcs_tagged_lock lock;

  auto tag = lock.optimistic_read();
  ASSERT_TRUE(lock.validate_optimistic_read(tag));
}

TEST_F(GcsTaggedLock, SuccessfulTryLock) {
  Gcs_tagged_lock lock;

  bool i_locked_it = lock.try_lock();
  ASSERT_TRUE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());
  lock.unlock();

  i_locked_it = lock.try_lock();
  ASSERT_TRUE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());
  lock.unlock();
}

TEST_F(GcsTaggedLock, UnsuccessfulOptimisticRead) {
  Gcs_tagged_lock lock;

  // Optimistic read finishes before unlock.
  auto tag = lock.optimistic_read();

  bool i_locked_it = lock.try_lock();
  ASSERT_TRUE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());

  bool successful = lock.validate_optimistic_read(tag);
  ASSERT_FALSE(successful);

  lock.unlock();

  // Optimistic read finishes after unlock.
  tag = lock.optimistic_read();

  i_locked_it = lock.try_lock();
  ASSERT_TRUE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());
  lock.unlock();

  successful = lock.validate_optimistic_read(tag);
  ASSERT_FALSE(successful);
}

TEST_F(GcsTaggedLock, UnsuccessfulTryLock) {
  Gcs_tagged_lock lock;

  bool i_locked_it = lock.try_lock();
  ASSERT_TRUE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());

  i_locked_it = lock.try_lock();
  ASSERT_FALSE(i_locked_it);
  ASSERT_TRUE(lock.is_locked());
  lock.unlock();
}

}  // namespace gcs_tagged_lock_unittest
