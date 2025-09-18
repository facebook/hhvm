/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/transport/rocket/core/RingBuffer.h>

namespace apache::thrift::rocket {

class RingBufferTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test fixture for non-trivial types
class NonTrivialType {
 public:
  NonTrivialType() : value(0) {}
  explicit NonTrivialType(int val) : value(val) {
    std::cout << "Constructor called" << " Current count: " << instanceCount
              << std::endl;
    instanceCount++;
  }

  NonTrivialType(const NonTrivialType& other) : value(other.value) {
    if (instanceCount > 0) {
      std::cout << "Copy constructor called"
                << " Current count: " << instanceCount << std::endl;
      instanceCount++;
    }
  }

  ~NonTrivialType() {
    if (instanceCount > 0) {
      std::cout << "Destructor called" << " Current count: " << instanceCount
                << std::endl;

      instanceCount--;
    }
  }

  int value;
  static int instanceCount;
};

int NonTrivialType::instanceCount = 0;

// Test basic construction
TEST_F(RingBufferTest, Construction) {
  // Valid log_size
  EXPECT_NO_THROW({
    RingBuffer<int> buffer(4); // Capacity = 2^4 = 16
    EXPECT_EQ(buffer.capacity(), 16);
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(buffer.empty());
  });

  // Invalid log_size (too small)
  EXPECT_THROW(RingBuffer<int>(0), std::invalid_argument);

  // Invalid log_size (too large)
  EXPECT_THROW(RingBuffer<int>(32), std::invalid_argument);
}

// Test basic operations: emplace_back, front, pop_front
TEST_F(RingBufferTest, BasicOperations) {
  RingBuffer<int> buffer(3); // Capacity = 2^3 = 8

  // Add elements
  EXPECT_TRUE(buffer.emplace_back(1));
  EXPECT_TRUE(buffer.emplace_back(2));
  EXPECT_TRUE(buffer.emplace_back(3));

  EXPECT_EQ(buffer.size(), 3);
  EXPECT_FALSE(buffer.empty());

  // Check front
  EXPECT_EQ(buffer.front(), 1);

  // Pop front
  buffer.pop_front();
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_EQ(buffer.front(), 2);

  buffer.pop_front();
  EXPECT_EQ(buffer.size(), 1);
  EXPECT_EQ(buffer.front(), 3);

  buffer.pop_front();
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_TRUE(buffer.empty());
}

// Test filling the buffer to capacity
TEST_F(RingBufferTest, FillToCapacity) {
  RingBuffer<int> buffer(2); // Capacity = 2^2 = 4

  // Fill to capacity
  EXPECT_TRUE(buffer.emplace_back(1));
  EXPECT_TRUE(buffer.emplace_back(2));
  EXPECT_TRUE(buffer.emplace_back(3));
  EXPECT_TRUE(buffer.emplace_back(4));

  // Buffer is now full
  EXPECT_EQ(buffer.size(), 4);
  EXPECT_FALSE(buffer.emplace_back(5)); // Should fail

  // Check elements
  EXPECT_EQ(buffer.front(), 1);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 2);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 3);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 4);
  buffer.pop_front();
  EXPECT_TRUE(buffer.empty());
}

// Test wrapping behavior
TEST_F(RingBufferTest, WrappingBehavior) {
  RingBuffer<int> buffer(2); // Capacity = 2^2 = 4

  // Fill buffer
  EXPECT_TRUE(buffer.emplace_back(1));
  EXPECT_TRUE(buffer.emplace_back(2));
  EXPECT_TRUE(buffer.emplace_back(3));
  EXPECT_TRUE(buffer.emplace_back(4));

  // Remove two elements
  buffer.pop_front();
  buffer.pop_front();
  EXPECT_EQ(buffer.size(), 2);

  // Add two more (these should wrap around in the internal array)
  EXPECT_TRUE(buffer.emplace_back(5));
  EXPECT_TRUE(buffer.emplace_back(6));
  EXPECT_EQ(buffer.size(), 4);

  // Check elements
  EXPECT_EQ(buffer.front(), 3);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 4);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 5);
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), 6);
  buffer.pop_front();
  EXPECT_TRUE(buffer.empty());
}

// Test consume method
TEST_F(RingBufferTest, ConsumeMethod) {
  RingBuffer<int> buffer(3); // Capacity = 2^3 = 8

  // Add elements
  for (int i = 1; i <= 5; i++) {
    EXPECT_TRUE(buffer.emplace_back(i));
  }

  // Consume 3 elements
  std::vector<int> consumed;
  size_t count =
      buffer.consume([&consumed](int& val) { consumed.push_back(val); }, 3);

  EXPECT_EQ(count, 3);
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_EQ(consumed.size(), 3);
  EXPECT_EQ(consumed[0], 1);
  EXPECT_EQ(consumed[1], 2);
  EXPECT_EQ(consumed[2], 3);

  // Consume remaining elements
  consumed.clear();
  count =
      buffer.consume([&consumed](int& val) { consumed.push_back(val); }, 10);

  EXPECT_EQ(count, 2);
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(consumed.size(), 2);
  EXPECT_EQ(consumed[0], 4);
  EXPECT_EQ(consumed[1], 5);
}

// Test with non-trivial types
TEST_F(RingBufferTest, NonTrivialTypes) {
  NonTrivialType::instanceCount = 0;
  {
    RingBuffer<NonTrivialType> buffer(2); // Capacity = 2^2 = 4

    // Add elements
    EXPECT_TRUE(buffer.emplace_back(10));
    EXPECT_TRUE(buffer.emplace_back(20));
    EXPECT_EQ(NonTrivialType::instanceCount, 2);

    // Check front
    EXPECT_EQ(buffer.front().value, 10);

    // Pop front
    buffer.pop_front();
    EXPECT_EQ(NonTrivialType::instanceCount, 1);
    EXPECT_EQ(buffer.front().value, 20);

    // Consume
    std::vector<int> values;
    buffer.consume(
        [&values](NonTrivialType& obj) { values.push_back(obj.value); }, 1);
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 20);
    EXPECT_EQ(NonTrivialType::instanceCount, 0);
  }
  // Buffer destructor should have been called
  EXPECT_EQ(NonTrivialType::instanceCount, 0);
}

// Test proper cleanup when buffer is destroyed with elements still in it
TEST_F(RingBufferTest, DestructorCleanup) {
  NonTrivialType::instanceCount = 0;
  {
    // Create a buffer and fill it to capacity
    RingBuffer<NonTrivialType> buffer(3); // Capacity = 2^3 = 8

    // Add elements to fill the buffer
    for (int i = 0; i < 8; i++) {
      EXPECT_TRUE(buffer.emplace_back(i * 10));
    }

    // Verify all elements were created
    EXPECT_EQ(NonTrivialType::instanceCount, 8);
    EXPECT_EQ(buffer.size(), 8);

    // Remove a few elements to create a "hole" in the circular buffer
    buffer.pop_front();
    buffer.pop_front();
    EXPECT_EQ(NonTrivialType::instanceCount, 6);

    // Add a few more to wrap around
    EXPECT_TRUE(buffer.emplace_back(80));
    EXPECT_TRUE(buffer.emplace_back(90));
    EXPECT_EQ(NonTrivialType::instanceCount, 8);

    // Buffer now has elements that wrap around in the internal array
    // Let it go out of scope without explicit cleanup
  }

  // Verify all elements were properly destroyed by the destructor
  EXPECT_EQ(NonTrivialType::instanceCount, 0);
}

// Test edge cases
TEST_F(RingBufferTest, EdgeCases) {
  // Empty buffer
  RingBuffer<int> buffer(1); // Capacity = 2^1 = 2
  EXPECT_TRUE(buffer.empty());

  // Consume on empty buffer
  std::vector<int> consumed;
  size_t count =
      buffer.consume([&consumed](int& val) { consumed.push_back(val); }, 5);
  EXPECT_EQ(count, 0);
  EXPECT_TRUE(consumed.empty());

  // Minimum capacity
  RingBuffer<int> minBuffer(1); // Capacity = 2^1 = 2
  EXPECT_EQ(minBuffer.capacity(), 2);
  EXPECT_TRUE(minBuffer.emplace_back(1));
  EXPECT_TRUE(minBuffer.emplace_back(2));
  EXPECT_FALSE(minBuffer.emplace_back(3)); // Should fail
}

// Test complex operations
TEST_F(RingBufferTest, ComplexOperations) {
  RingBuffer<std::string> buffer(2); // Capacity = 2^2 = 4

  // Add elements
  EXPECT_TRUE(buffer.emplace_back("one"));
  EXPECT_TRUE(buffer.emplace_back("two"));
  EXPECT_TRUE(buffer.emplace_back("three"));
  EXPECT_TRUE(buffer.emplace_back("four"));

  // Buffer is now full
  EXPECT_FALSE(buffer.emplace_back("five"));

  // Remove and add in cycles to test wrapping
  buffer.pop_front();
  EXPECT_TRUE(buffer.emplace_back("five"));

  buffer.pop_front();
  EXPECT_TRUE(buffer.emplace_back("six"));

  // Check elements
  EXPECT_EQ(buffer.front(), "three");
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), "four");
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), "five");
  buffer.pop_front();
  EXPECT_EQ(buffer.front(), "six");
  buffer.pop_front();
  EXPECT_TRUE(buffer.empty());
}

// Test performance with large number of operations
TEST_F(RingBufferTest, PerformanceTest) {
  RingBuffer<int> buffer(10); // Capacity = 2^10 = 1024

  // Add many elements
  for (int i = 0; i < 1000; i++) {
    EXPECT_TRUE(buffer.emplace_back(i));
  }

  EXPECT_EQ(buffer.size(), 1000);

  // Consume in batches
  int sum = 0;
  int expectedSum = 0;
  for (int i = 0; i < 1000; i++) {
    expectedSum += i;
  }

  // Consume in batches of 100
  for (int i = 0; i < 10; i++) {
    buffer.consume([&sum](int& val) { sum += val; }, 100);
  }

  EXPECT_EQ(sum, expectedSum);
  EXPECT_TRUE(buffer.empty());
}

} // namespace apache::thrift::rocket
