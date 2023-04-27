/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All Rights Reserved.

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
#include <array> /* std::array */
#include <new>   /* std::bad_alloc */
#include <thread>

#include "storage/temptable/include/temptable/allocator.h" /* temptable::Allocator */
#include "storage/temptable/include/temptable/constants.h" /* temptable::ALLOCATOR_MAX_BLOCK_BYTES */

namespace temptable_allocator_unittest {

// A "probe" which gains us read-only access to temptable::MemoryMonitor.
// Necessary for implementing certain unit-tests.
struct MemoryMonitorReadOnlyProbe : private temptable::MemoryMonitor {
  static size_t ram_consumption() {
    return temptable::MemoryMonitor::ram_consumption();
  }
};

TEST(temptable_allocator, basic) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      constexpr size_t n_allocate = 128;
      std::array<uint8_t *, n_allocate> a;
      constexpr size_t n_elements = 16;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      for (size_t i = 0; i < n_allocate; ++i) {
        a[i] = allocator.allocate(n_elements);

        for (size_t j = 0; j < n_elements; ++j) {
          a[i][j] = 0xB;
        }
      }

      EXPECT_FALSE(temptable::shared_block.is_empty());

      for (size_t i = 0; i < n_allocate; ++i) {
        allocator.deallocate(a[i], n_elements);
      }
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

TEST(temptable_allocator, shared_block_is_kept_after_last_deallocation) {
  {
    bool old_val = temptable_track_shared_block_ram;
    temptable_track_shared_block_ram = false;
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      uint8_t *ptr = allocator.allocate(16);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      allocator.deallocate(ptr, 16);
      EXPECT_FALSE(temptable::shared_block.is_empty());
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
    temptable_track_shared_block_ram = old_val;
  }
}

TEST(temptable_allocator, rightmost_chunk_deallocated_reused_for_allocation) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // Allocate first Chunk which is less than the 1MB
      size_t first_chunk_size = 512 * 1024;
      uint8_t *first_chunk = allocator.allocate(first_chunk_size);

      // Calculate and allocate second chunk in such a way that
      // it lies within the block and fills it
      size_t first_chunk_actual_size =
          temptable::Chunk::size_hint(first_chunk_size);
      size_t space_left_in_block =
          temptable::shared_block.size() -
          temptable::Block::size_hint(first_chunk_actual_size);
      size_t second_chunk_size =
          space_left_in_block - (first_chunk_actual_size - first_chunk_size);
      uint8_t *second_chunk = allocator.allocate(second_chunk_size);

      // Make sure that pointers (Chunk's) are from same blocks
      EXPECT_EQ(temptable::Block(temptable::Chunk(first_chunk)),
                temptable::Block(temptable::Chunk(second_chunk)));

      EXPECT_FALSE(temptable::shared_block.can_accommodate(1));

      // Deallocate Second Chunk
      allocator.deallocate(second_chunk, second_chunk_size);

      // Allocate Second Chunk again
      second_chunk = allocator.allocate(second_chunk_size);

      // Make sure that pointers (Chunk's) are from same blocks
      EXPECT_EQ(temptable::Block(temptable::Chunk(first_chunk)),
                temptable::Block(temptable::Chunk(second_chunk)));

      // Deallocate Second Chunk
      allocator.deallocate(second_chunk, second_chunk_size);

      // Deallocate First Chunk
      allocator.deallocate(first_chunk, first_chunk_size);
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

TEST(temptable_allocator, ram_consumption_is_not_monitored_for_shared_blocks) {
  {
    bool old_val = temptable_track_shared_block_ram;
    temptable_track_shared_block_ram = false;
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // First allocation is fed from shared-block
      size_t shared_block_n_elements = 1024 * 1024;
      uint8_t *shared_block = allocator.allocate(shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      // RAM consumption is still 0
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // Deallocate the shared-block
      allocator.deallocate(shared_block, shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
    temptable_track_shared_block_ram = old_val;
  }
}

TEST(temptable_allocator,
     ram_consumption_drops_to_zero_when_non_shared_block_is_destroyed) {
  {
    bool old_val = temptable_track_shared_block_ram;
    temptable_track_shared_block_ram = false;
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // Make sure we fill up the shared_block first
      // nr of elements must be >= 1MiB in size
      size_t shared_block_n_elements = 1024 * 1024 + 256;
      uint8_t *shared_block = allocator.allocate(shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      // Not even 1-byte should be able to fit anymore
      EXPECT_FALSE(temptable::shared_block.can_accommodate(1));

      // Now our next allocation should result in new block allocation ...
      size_t non_shared_block_n_elements = 2 * 1024;
      uint8_t *non_shared_block =
          allocator.allocate(non_shared_block_n_elements);

      // Make sure that pointers (Chunk's) are from different blocks
      EXPECT_NE(temptable::Block(temptable::Chunk(non_shared_block)),
                temptable::Block(temptable::Chunk(shared_block)));

      // RAM consumption should be greater or equal than
      // non_shared_block_n_elements bytes at this point
      EXPECT_GE(MemoryMonitorReadOnlyProbe::ram_consumption(),
                non_shared_block_n_elements);

      // Deallocate the non-shared block
      allocator.deallocate(non_shared_block, non_shared_block_n_elements);

      // RAM consumption must drop to 0
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // Deallocate the shared-block
      allocator.deallocate(shared_block, shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
    temptable_track_shared_block_ram = old_val;
  }
}

TEST(temptable_allocator, shared_block_is_destroyed_after_last_deallocation) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      uint8_t *ptr = allocator.allocate(16);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      allocator.deallocate(ptr, 16);
      EXPECT_TRUE(temptable::shared_block.is_empty());
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

TEST(temptable_allocator, ram_consumption_is_monitored_for_shared_blocks) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // First allocation is fed from shared-block
      size_t shared_block_n_elements = 1024 * 1024;
      uint8_t *shared_block = allocator.allocate(shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      // RAM consumption has increased.
      EXPECT_GE(MemoryMonitorReadOnlyProbe::ram_consumption(),
                shared_block_n_elements);

      // Deallocate the shared-block
      allocator.deallocate(shared_block, shared_block_n_elements);
      EXPECT_TRUE(temptable::shared_block.is_empty());

      // RAM consumption is 0 after deallocation from shared block
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // Reallocate yet again
      shared_block = allocator.allocate(shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      // RAM consumption has increased.
      EXPECT_GE(MemoryMonitorReadOnlyProbe::ram_consumption(),
                shared_block_n_elements);

      // Deallocate the shared-block
      allocator.deallocate(shared_block, shared_block_n_elements);
      EXPECT_TRUE(temptable::shared_block.is_empty());

      // RAM consumption is 0 after deallocation from shared block
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

TEST(temptable_allocator, ram_consumption_for_all_blocks) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      // RAM consumption is 0 at the start
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);

      // Make sure we fill up the shared_block first
      // nr of elements must be >= 1MiB in size
      size_t shared_block_n_elements = 1024 * 1024 + 256;
      uint8_t *shared_block = allocator.allocate(shared_block_n_elements);
      EXPECT_FALSE(temptable::shared_block.is_empty());

      // Not even 1-byte should be able to fit anymore
      EXPECT_FALSE(temptable::shared_block.can_accommodate(1));

      // Now our next allocation should result in new block allocation ...
      size_t non_shared_block_n_elements = 2 * 1024;
      uint8_t *non_shared_block =
          allocator.allocate(non_shared_block_n_elements);

      // Make sure that pointers (Chunk's) are from different blocks
      EXPECT_NE(temptable::Block(temptable::Chunk(non_shared_block)),
                temptable::Block(temptable::Chunk(shared_block)));

      // RAM consumption should be greater or equal than
      // both shared and non_shared_block_n_elements bytes at this point
      EXPECT_GE(MemoryMonitorReadOnlyProbe::ram_consumption(),
                shared_block_n_elements + non_shared_block_n_elements);

      // Deallocate the shared-block
      allocator.deallocate(shared_block, shared_block_n_elements);
      EXPECT_TRUE(temptable::shared_block.is_empty());

      // RAM consumption should be greater or equal than
      // non_shared_block_n_elements bytes at this point
      EXPECT_GE(MemoryMonitorReadOnlyProbe::ram_consumption(),
                non_shared_block_n_elements);

      // Deallocate the non-shared block
      allocator.deallocate(non_shared_block, non_shared_block_n_elements);

      // RAM consumption must drop to 0
      EXPECT_EQ(MemoryMonitorReadOnlyProbe::ram_consumption(), 0);
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

TEST(temptable_allocator, edge) {
  temptable::Allocator<uint8_t> allocator;

  using namespace temptable;

  EXPECT_EQ(nullptr, allocator.allocate(0));

#ifndef DBUG_OFF
  DBUG_SET("+d,temptable_allocator_oom");
  bool thrown = false;
  try {
    allocator.allocate(8_MiB);
  } catch (...) {
    thrown = true;
  }
  EXPECT_EQ(true, thrown);
  DBUG_SET("-d,temptable_allocator_oom");
#endif /* DBUG_OFF */
}

TEST(temptable_allocator, block_size_cap) {
  {
    EXPECT_TRUE(temptable::shared_block.is_empty());
    std::thread t([]() {
      temptable::Allocator<uint8_t> allocator;

      using namespace temptable;

      constexpr size_t alloc_size = 1_MiB;
      constexpr size_t n_allocate = ALLOCATOR_MAX_BLOCK_BYTES / alloc_size + 10;
      std::array<uint8_t *, n_allocate> a;

      for (size_t i = 0; i < n_allocate; ++i) {
        a[i] = allocator.allocate(alloc_size);
      }

      EXPECT_FALSE(temptable::shared_block.is_empty());

      for (size_t i = 0; i < n_allocate; ++i) {
        allocator.deallocate(a[i], alloc_size);
      }
    });
    t.join();
    EXPECT_TRUE(temptable::shared_block.is_empty());
  }
}

} /* namespace temptable_allocator_unittest */
