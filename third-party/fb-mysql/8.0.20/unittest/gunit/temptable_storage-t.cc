/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <thread>

#include "storage/temptable/include/temptable/allocator.h" /* temptable::Allocator */
#include "storage/temptable/include/temptable/storage.h" /* temptable::Storage */

namespace temptable_storage_unittest {

TEST(StorageTest, Iterate) {
  std::thread t([]() {
    temptable::Allocator<uint8_t> allocator;
    temptable::Storage storage(&allocator);

    storage.element_size(sizeof(uint64_t));

    for (uint64_t i = 0; i < 10000; ++i) {
      *static_cast<uint64_t *>(storage.allocate_back()) = i;
    }

    uint64_t i = 0;
    for (auto it = storage.begin(); it != storage.end(); ++it, ++i) {
      EXPECT_EQ(i, *static_cast<uint64_t *>(*it));
    }

    i = storage.size();
    auto it = storage.end();
    for (; it != storage.begin();) {
      --it;
      --i;
      EXPECT_EQ(i, *static_cast<uint64_t *>(*it));
    }
    EXPECT_EQ(0u, i);
  });
  t.join();
}

TEST(StorageTest, AllocatorRebind) {
  // Bug in VS2019 error C3409 if we do the same as above.
  // Turns out it is the rebind which confuses the compiler.
  auto thread_function = []() {
    temptable::Allocator<uint8_t> alloc;
    uint8_t *shared_eater = alloc.allocate(
        1048576);  // Make sure to consume the initial shared block.
    uint8_t *ptr = alloc.allocate(100);

    decltype(alloc)::rebind<uint32_t>::other rebound_alloc(alloc);

    alloc.deallocate(ptr, 100);

    uint32_t *ptr2 = rebound_alloc.allocate(50);
    rebound_alloc.deallocate(ptr2, 50);

    alloc.deallocate(shared_eater, 1048576);
  };
  std::thread t(thread_function);
  t.join();
}

} /* namespace temptable_storage_unittest */
