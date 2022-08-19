/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <algorithm>
#include <array>
#include <atomic>
#include <iostream>
#include <numeric>
#include <thread>
#include <unordered_set>
#include <vector>
#include "gcs_base_test.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_mpsc_queue.h"

namespace gcs_xcom_mpsc_queue_unittest {

class GcsMpscQueueTest : public GcsBaseTest {
 protected:
  GcsMpscQueueTest() {}

  virtual void SetUp() {}

  virtual void TearDown() {}
};

static void producer_function(Gcs_mpsc_queue<int> &queue,
                              std::vector<int> &sequence,
                              std::atomic<int> &nr_actors_ready,
                              int const &nr_actors) {
  nr_actors_ready.fetch_add(1, std::memory_order_relaxed);
  while (nr_actors_ready.load(std::memory_order_relaxed) != nr_actors)
    ;
  for (int &value : sequence) {
    queue.push(&value);
  }
}

static std::vector<int> setup_sequence(int const &nr_elements) {
  std::vector<int> sequence(nr_elements);
  std::iota(sequence.begin(), sequence.end(), 1);
  return sequence;
}

static std::vector<std::thread> start_producers(
    int const &nr_producers, std::atomic<int> &nr_actors_ready,
    int const &nr_actors, Gcs_mpsc_queue<int> &queue,
    std::vector<int> &sequence) {
  std::vector<std::thread> producers(nr_producers);
  nr_actors_ready.store(0, std::memory_order_relaxed);
  for (auto i = 0; i < nr_producers; i++) {
    producers[i] =
        std::thread(producer_function, std::ref(queue), std::ref(sequence),
                    std::ref(nr_actors_ready), std::ref(nr_actors));
  }
  return producers;
}

static void wait_for_producers(std::vector<std::thread> &producers) {
  for (std::thread &producer : producers) {
    producer.join();
  }
}

static void validate(int const &nr_producers, std::vector<int> &sequence,
                     std::vector<int> &consumed) {
  // Validate that consumed elements respect per-producer FIFO order.
  std::vector<std::vector<int>::iterator> cursors(nr_producers);
  for (auto &cursor : cursors) {
    cursor = sequence.begin();
  }
  for (int const &value : consumed) {
    auto cursor = std::find_if(
        cursors.begin(), cursors.end(),
        [&sequence, &value](std::vector<int>::iterator cursor_arg) {
          return (cursor_arg != sequence.end() && *cursor_arg == value);
        });
    ASSERT_NE(cursor, cursors.end());
    (*cursor)++;
  }
  // Validate nr. of consumed elements. All cursors at the end means we
  // processed all the elements of all producers.
  for (auto cursor : cursors) {
    ASSERT_EQ(cursor, sequence.end());
  }
}

static void test_concurrent_producers_sequential_consumer(
    int const nr_producers, int const nr_elements) {
  std::atomic<int> nr_producers_ready;
  Gcs_mpsc_queue<int> queue;

  std::vector<int> sequence = setup_sequence(nr_elements);

  std::vector<std::thread> producers = start_producers(
      nr_producers, nr_producers_ready, nr_producers, queue, sequence);

  wait_for_producers(producers);

  // Create sequence of consumed elements.
  std::vector<int> consumed;
  for (auto *payload = queue.pop(); payload != nullptr; payload = queue.pop()) {
    consumed.push_back(*payload);
  }

  validate(nr_producers, sequence, consumed);
}

/* A single producer enqueues elements. After it finishes, a single consumer
 * dequeues the elements and verifies FIFO order. */
TEST_F(GcsMpscQueueTest, SequentialTest) {
  int constexpr nr_producers = 1;
  int constexpr nr_elements = 32768;
  test_concurrent_producers_sequential_consumer(nr_producers, nr_elements);
}

/* A set of producers enqueue elements concurrently. After they finish, a single
 * consumer dequeues the elements and verifies per-producer FIFO order. */
TEST_F(GcsMpscQueueTest, MultipleConcurrentProducersSequentialConsumerTest) {
  int constexpr nr_producers = 4;
  int constexpr nr_elements = 32768;
  test_concurrent_producers_sequential_consumer(nr_producers, nr_elements);
}

static void test_concurrent_producers_concurrent_consumer(
    int const nr_producers, int const nr_elements) {
  std::size_t const total_nr_elements = nr_producers * nr_elements;
  int const nr_actors = nr_producers + 1;
  std::atomic<int> nr_actors_ready;
  Gcs_mpsc_queue<int> queue;

  std::vector<int> sequence = setup_sequence(nr_elements);

  std::vector<std::thread> producers = start_producers(
      nr_producers, nr_actors_ready, nr_actors, queue, sequence);

  // Consume queue concurrently with producers.
  nr_actors_ready.fetch_add(1, std::memory_order_relaxed);
  while (nr_actors_ready.load(std::memory_order_relaxed) != nr_actors)
    ;
  std::vector<int> consumed;
  for (auto *payload = queue.pop(); consumed.size() < total_nr_elements;
       payload = queue.pop()) {
    if (payload != nullptr) {
      consumed.push_back(*payload);
    }
  }

  wait_for_producers(producers);

  validate(nr_producers, sequence, consumed);
}

/* A set of producers enqueue elements, while a single consumer dequeues the
 * elements. Everyone is concurrent with each other. After the consumer has
 * dequeued all the elements, it verifies per-producer FIFO order. */
TEST_F(GcsMpscQueueTest, MultipleConcurrentProducersConcurrentConsumerTest) {
  int constexpr nr_producers = 4;
  int constexpr nr_elements = 32768;
  test_concurrent_producers_concurrent_consumer(nr_producers, nr_elements);
}

}  // namespace gcs_xcom_mpsc_queue_unittest
