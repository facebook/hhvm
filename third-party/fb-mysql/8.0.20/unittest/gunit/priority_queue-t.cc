/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <gtest/gtest.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <vector>
#include "my_config.h"

#include "priority_queue.h"

namespace priority_queue_unittest {

// random integer generator -- start
template <typename IntegerType>
class random_integer_generator {
 public:
  random_integer_generator(unsigned int seed = 0) { srand(seed); }

  IntegerType operator()(IntegerType const &low, IntegerType const &high) {
    IntegerType i = rand() % (high - low + 1);
    i += low;
    return i;
  }
};
// random integer generator -- end

//--------------------------------------------------------

// handle support -- start
template <typename T>
struct handle {
  T *ptr;

  handle() : ptr(NULL) {}
  handle(T &t) : ptr(&t) {}
};

template <typename T>
inline std::ostream &operator<<(std::ostream &os, handle<T> const &h) {
  os << *h.ptr;
  return os;
}

template <typename Handle, typename Value, typename Less = std::less<Value>>
struct handle_less {
  inline bool operator()(Handle const &p1, Handle const &p2) const {
    return Less()(*p1.ptr, *p2.ptr);
  }
};
// handle support -- end

//--------------------------------------------------------

// dummy stream that "eats" all input
struct null_stream : public std::ostream {
  // Visual Studio needs a default constructor.
  null_stream() : std::ostream(nullptr) {}
};

template <typename T>
inline null_stream &operator<<(null_stream &ns, T const &) {
  return ns;
}

//--------------------------------------------------------

// i/o support -- start
template <typename InputIterator>
inline void print_range(std::ostream &os, InputIterator first,
                        InputIterator last) {
  for (InputIterator it = first; it != last; ++it) {
    os << " " << *it;
  }
  os << std::endl;
}

template <typename InputIterator>
inline void print_range(std::ostream &os, std::string const &header,
                        InputIterator first, InputIterator last) {
  os << header;
  print_range(os, first, last);
}
// i/o support -- end

//--------------------------------------------------------

// Some template functions below do not play well with gtest internals
static void assert_true_helper(bool val) { ASSERT_TRUE(val); }

// k min elements algorithm -- start
template <bool UseSorting>
struct min_k_elements {
  template <typename InputIterator, typename OutputIterator>
  static inline OutputIterator apply(InputIterator first, InputIterator last,
                                     size_t k, OutputIterator oit) {
    SCOPED_TRACE("");
    assert_true_helper(static_cast<size_t>(std::distance(first, last)) > k);

    typedef typename std::iterator_traits<InputIterator>::value_type value_type;

    std::vector<value_type> values;

    // copy locally
    std::copy(first, last, std::back_inserter(values));

    // sort
    std::sort(values.begin(), values.end());

    // output k smallest values
    std::copy(values.begin(), values.begin() + k, oit);
    return oit;
  }
};

template <>
struct min_k_elements<false> {
  template <typename RandomAccessIterator, typename OutputIterator>
  static inline OutputIterator apply(RandomAccessIterator first,
                                     RandomAccessIterator last, size_t k,
                                     OutputIterator oit) {
    SCOPED_TRACE("");
    assert_true_helper(static_cast<size_t>(std::distance(first, last)) > k);

    typedef typename std::iterator_traits<RandomAccessIterator>::value_type
        value_type;

    typedef handle<value_type> handle_type;

    typedef handle_less<handle_type, value_type, std::less<value_type>>
        handle_less_type;

    typedef Priority_queue<handle_type, std::vector<handle_type>,
                           handle_less_type>
        queue_type;

    typedef typename queue_type::const_iterator queue_iterator;

    // copy k + 1 values locally
    std::vector<value_type> values;
    values.reserve(k + 1);

    std::copy(first, first + k + 1, std::back_inserter(values));

    // create a queue of k + 1 handles to the local values
    queue_type queue;
    assert_true_helper(!queue.reserve(k + 1));

    for (size_t i = 0; i < k + 1; ++i) {
      EXPECT_FALSE(queue.push(handle_type(values[i])));
    }
    SCOPED_TRACE("");
    assert_true_helper(queue.is_valid());

    // for the remaining values update the queue's root node
    // and rebuild the heap
    for (RandomAccessIterator it = first + k + 1; it != last; ++it) {
      *queue[0].ptr = *it;
      queue.update(0);
    }

    // output the k minimum values (all but the root)
    for (queue_iterator it = ++queue.begin(); it != queue.end(); ++it) {
      *oit++ = *it->ptr;
    }

    return oit;
  }
};
// k min elements algorithm -- end

//--------------------------------------------------------

template <typename RandomAccessIterator>
inline void test_min_k_elements(RandomAccessIterator first,
                                RandomAccessIterator last, size_t k) {
  std::vector<int> keys_copy(first, last);
  std::sort(keys_copy.begin(), keys_copy.end());

#ifdef PRIORITY_QUEUE_TEST_DEBUG
  std::ostream &os = std::cout;
#else
  null_stream os;
#endif

  print_range(os, "elements: ", first, last);

  print_range(os, "sorted elements: ", keys_copy.begin(), keys_copy.end());
  os << std::endl;

  std::vector<int> min_elements_sort, min_elements_heap;

  // using the heap
  min_k_elements<false>::apply(first, last, k,
                               std::back_inserter(min_elements_heap));

  os << "min " << k << " elements (heap): ";
  print_range(os, min_elements_heap.begin(), min_elements_heap.end());

  std::sort(min_elements_heap.begin(), min_elements_heap.end());
  os << "min " << k << " elements (hp/s): ";
  print_range(os, min_elements_heap.begin(), min_elements_heap.end());
  os << std::endl;

  // using sorting
  min_k_elements<true>::apply(first, last, k,
                              std::back_inserter(min_elements_sort));

  os << "min " << k << " elements (sort): ";
  print_range(os, min_elements_sort.begin(), min_elements_sort.end());
  os << std::endl;

  ASSERT_TRUE(std::equal(min_elements_sort.begin(), min_elements_sort.end(),
                         min_elements_heap.begin()));
}

//--------------------------------------------------------

template <typename Queue>
inline void print_update_msg(std::ostream &os, std::string header,
                             Queue const &q, typename Queue::size_type pos,
                             typename Queue::value_type const &old_value,
                             typename Queue::value_type const &new_value) {
  os << header << " priority of element " << old_value << " at position " << pos
     << " to new value " << new_value << "; new_queue: " << q << std::endl;
}

template <typename RandomAccessIterator>
inline void test_heap(RandomAccessIterator first, RandomAccessIterator last) {
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type
      value_type;

  typedef Priority_queue<value_type> queue;
  typedef typename queue::size_type size_type;
  typedef typename queue::iterator iterator;
  typedef typename queue::const_iterator const_iterator;

#ifdef PRIORITY_QUEUE_TEST_DEBUG
  std::ostream &os = std::cout;
#else
  null_stream os;
#endif

  queue pq;

  for (RandomAccessIterator it = first; it != last; ++it) {
    EXPECT_FALSE(pq.push(*it));
    ASSERT_TRUE(pq.is_valid());
    os << "queue: " << pq << std::endl;
  }

  // test constructor with iterators
  {
    queue pq2(first, last);
    ASSERT_TRUE(pq2.is_valid());
    os << "queue (constructor with iterators): " << pq2 << std::endl;
  }

  // test top
  os << "top element: " << pq.top() << std::endl;
  ASSERT_TRUE(pq.top() == pq[0]);

  // test push
  value_type new_value = *std::min_element(pq.begin(), pq.end()) - 1;
  EXPECT_FALSE(pq.push(new_value));
  ASSERT_TRUE(pq.is_valid());
  os << "pushed " << new_value << "; new queue: " << pq << std::endl;

  // test pop
  pq.pop();
  ASSERT_TRUE(pq.is_valid());
  os << "popped top element; new queue: " << pq << std::endl;

  // test decrease
  // decrease priority of element at position 3
  {
    size_type pos = 3;
    value_type old_priority = pq[pos];
    value_type new_priority = pq[pos];
    pq.decrease(pos, new_priority);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "decreasing (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos];
    *(pq.begin() + pos) = new_priority;
    pq.decrease(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "decreasing (1)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] - 10;
    pq.decrease(pos, new_priority);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "decreasing (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] - 20;
    pq[pos] = new_priority;
    pq.decrease(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "decreasing (1)", pq, pos, old_priority, new_priority);
  }

  // test increase
  // increase priority of element at position 4
  {
    size_type pos = 4;
    value_type old_priority = pq[pos];
    value_type new_priority = pq[pos];
    pq.increase(pos, new_priority);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "increasing (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos];
    *(pq.begin() + pos) = new_priority;
    pq.increase(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "increasing (1)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] + 30;
    pq.increase(pos, new_priority);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "increasing (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] + 50;
    pq[pos] = new_priority;
    pq.increase(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "increasing (1)", pq, pos, old_priority, new_priority);
  }

  // test update
  // update priority of element at position 2
  {
    size_type pos = 2;
    value_type old_priority = pq[pos];
    value_type new_priority = pq[pos];
    pq.update(pos, new_priority);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "updating (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos];
    *(pq.begin() + pos) = new_priority;
    pq.update(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "updating (1)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] - 20;
    *(pq.begin() + pos) = new_priority;
    pq.update(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "updating (2)", pq, pos, old_priority, new_priority);

    old_priority = pq[pos];
    new_priority = pq[pos] + 100;
    pq[pos] = new_priority;
    pq.update(pos);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "updating (1)", pq, pos, old_priority, new_priority);
  }

  // test size
  ASSERT_TRUE(pq.size() == static_cast<size_t>(std::distance(first, last)));
  ASSERT_TRUE(pq.is_valid());
  os << "size: " << pq.size() << std::endl;

  // test empty
  ASSERT_TRUE(!pq.empty());
  ASSERT_TRUE(pq.is_valid());
  os << "empty? " << std::boolalpha << pq.empty() << std::noboolalpha
     << std::endl;

  // print all elements in queue using operator[]
  os << "queue (using operator[]):";
  for (size_type i = 0; i < pq.size(); ++i) {
    os << " " << pq[i];
  }
  os << std::endl;

  // print all elements in queue using const/non-const iterators
  os << "queue (using const iterators):";
  for (const_iterator it = pq.begin(); it != pq.end(); ++it) {
    os << " " << *it;
  }
  os << std::endl;

  os << "queue (using non-const iterators):";
  for (iterator it = pq.begin(); it != pq.end(); ++it) {
    os << " " << *it;
  }
  os << std::endl;

  // testing swap
  queue other;

  ASSERT_TRUE(pq.is_valid() && !pq.empty());
  ASSERT_TRUE(other.is_valid() && other.empty());
  os << "before swap: " << pq << "; " << other << std::endl;
  pq.swap(other);
  ASSERT_TRUE(pq.is_valid() && pq.empty());
  ASSERT_TRUE(other.is_valid() && !other.empty());
  os << "after swap: " << pq << "; " << other << std::endl;

  // clear the heap
  pq.clear();
  ASSERT_TRUE(pq.is_valid());
  ASSERT_TRUE(pq.empty());
  os << "clearing the queue" << std::endl;

  // reserve 10 heap elements and push 10 elements in the heap
  os << "testing reserve" << std::endl;
  ASSERT_TRUE(pq.empty());
  ASSERT_FALSE(pq.reserve(2 * pq.capacity()));
  ASSERT_TRUE(pq.empty());
  for (size_type i = 0; i < static_cast<size_t>(std::distance(first, last));
       ++i) {
    ASSERT_TRUE(pq.size() == i);
    EXPECT_FALSE(pq.push(*(last - 1 - i)));
    ASSERT_TRUE(pq.is_valid());
    os << "queue: " << pq << std::endl;
  }

  // now use the non-const top() method to update the root element
  os << "testing non-const top()" << std::endl;
  {
    value_type old_priority = pq.top();
    value_type new_priority = pq.top() - 40;
    pq.top() = pq.top() - 40;
    pq.update(0);
    ASSERT_TRUE(pq.is_valid());
    print_update_msg(os, "updating (using top)", pq, 0, old_priority,
                     new_priority);
  }
}

//--------------------------------------------------------

template <typename RandomAccessIterator>
inline void test_heap_of_handles(RandomAccessIterator first,
                                 RandomAccessIterator last) {
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type
      value_type;

  typedef handle<value_type> handle_type;

  typedef handle_less<handle_type, value_type, std::greater<value_type>>
      handle_greater_type;

  typedef Priority_queue<handle_type, std::vector<handle_type>,
                         handle_less<handle_type, value_type>>
      queue;

  typedef typename queue::const_iterator const_iterator;

#ifdef PRIORITY_QUEUE_TEST_DEBUG
  std::ostream &os = std::cout;
#else
  null_stream os;
#endif

  size_t const N = static_cast<size_t>(std::distance(first, last));

  std::vector<handle_type> handles;
  for (size_t i = 0; i < N; ++i) {
    handles.push_back(handle_type(*(first + i)));
  }
  for (size_t i = 0; i < N; ++i) {
    os << handles[i] << std::endl;
  }

  queue pq;
  for (size_t i = 0; i < N; ++i) {
    EXPECT_FALSE(pq.push(handles[i]));
    ASSERT_TRUE(pq.is_valid());
    os << "queue: " << pq << std::endl;
  }

  {
    queue pq2(handles.begin(), handles.end());
    ASSERT_TRUE(pq2.is_valid());
    os << "queue: " << pq2 << std::endl;
  }

  const_iterator it_max =
      std::max_element(pq.begin(), pq.end(), handle_greater_type());
  *pq[0].ptr = *it_max->ptr + 1000;
  os << "queue: " << pq << std::endl;
  pq.update_top();
  ASSERT_TRUE(pq.is_valid());
  os << "queue: " << pq << std::endl;

  it_max = std::max_element(pq.begin(), pq.end(), handle_greater_type());
  *pq[0].ptr = *it_max->ptr + 500;
  os << "queue: " << pq << std::endl;
  pq.update_top();
  ASSERT_TRUE(pq.is_valid());
  os << "queue: " << pq << std::endl;
}

//--------------------------------------------------------

class PriorityQueueTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    int xkeys[10] = {10, 4, 7, 8, 21, -5, 6, 10, 7, 9};
    memcpy(keys, xkeys, sizeof(xkeys));
    pq = Priority_queue<int>(xkeys, xkeys + 10);
    keys2.assign(5, 50);
  }

  size_t parent(size_t i) { return Priority_queue<int>::parent(i); }
  size_t left(size_t i) { return Priority_queue<int>::left(i); }
  size_t right(size_t i) { return Priority_queue<int>::right(i); }

  int keys[10];
  Priority_queue<int> pq;
  std::vector<unsigned> keys2;
};

TEST_F(PriorityQueueTest, ParentLeftRight) {
  EXPECT_EQ(0U, parent(1));
  EXPECT_EQ(0U, parent(2));
  EXPECT_EQ(1U, left(0));
  EXPECT_EQ(2U, right(0));
  for (size_t ix = 0; ix < 42; ++ix) {
    EXPECT_EQ(ix, parent(left(ix)));
    EXPECT_EQ(ix, parent(right(ix)));
    EXPECT_EQ(1 + left(ix), right(ix));
  }
}

struct My_less {
  bool operator()(int a, int b) const { return a < b; }
};

struct My_greater {
  bool operator()(int a, int b) const { return a > b; }
};

TEST_F(PriorityQueueTest, MaxVsMinHeap) {
  Priority_queue<int, std::vector<int>, My_less> pql;
  Priority_queue<int, std::vector<int>, My_greater> pqg;
  for (int ix = 0; ix < 10; ++ix) {
    EXPECT_FALSE(pql.push(ix));
    EXPECT_FALSE(pqg.push(ix));
    EXPECT_EQ(ix, pql.top());
    EXPECT_EQ(0, pqg.top());
    EXPECT_TRUE(pql.is_valid());
    EXPECT_TRUE(pqg.is_valid());
  }
  std::stringstream ss1, ss2;
  ss1 << pql;
  EXPECT_STREQ("9 8 5 6 7 1 4 0 3 2 ", ss1.str().c_str());
  ss2 << pqg;
  EXPECT_STREQ("0 1 2 3 4 5 6 7 8 9 ", ss2.str().c_str());
}

TEST_F(PriorityQueueTest, DifferentCtors) {
  Priority_queue<int, std::vector<int>, My_less> pql;
  std::vector<int> intvec;
  for (int ix = 0; ix < 10; ++ix) {
    Priority_queue<int> pq_ix(intvec.begin(), intvec.end());
    EXPECT_TRUE(pq_ix.is_valid());
    EXPECT_FALSE(pql.push(ix));
    EXPECT_EQ(ix, pql.top());
    intvec.push_back(ix);
  }
  Priority_queue<int, std::vector<int>, My_less> pql_range(intvec.begin(),
                                                           intvec.end());

  EXPECT_EQ(10U, pql.size());
  EXPECT_FALSE(pql.empty());
  EXPECT_TRUE(pql.is_valid());
  EXPECT_EQ(10U, pql_range.size());
  EXPECT_FALSE(pql_range.empty());
  EXPECT_TRUE(pql_range.is_valid());

  std::stringstream ss1, ss2;
  ss1 << pql;
  ss2 << pql_range;
  // Different heaps, both are valid:
  EXPECT_STREQ("9 8 5 6 7 1 4 0 3 2 ", ss1.str().c_str());
  EXPECT_STREQ("9 8 6 7 4 5 2 0 3 1 ", ss2.str().c_str());
}

TEST_F(PriorityQueueTest, Swap) {
  std::random_device rng;
  std::mt19937 urng(rng());
  std::shuffle(keys, keys + 10, urng);
  Priority_queue<int> pq(keys, keys + 10);
  std::stringstream ss1, ss2;
  ss1 << pq;
  Priority_queue<int> pq_swap;
  pq_swap.swap(pq);
  ss2 << pq_swap;
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
  EXPECT_EQ(0U, pq.size());
}

TEST_F(PriorityQueueTest, DecreaseNoop) {
  std::stringstream ss1, ss2;
  ss1 << pq;
  for (size_t ix = 0; ix < 10; ++ix) {
    pq.decrease(ix);
    int val = pq[ix];
    pq.decrease(ix, val);
    *(pq.begin() + ix) = val;
    pq.decrease(ix);
    EXPECT_TRUE(pq.is_valid());
  }
  ss2 << pq;
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, IncreaseNoop) {
  std::stringstream ss1, ss2;
  ss1 << pq;
  for (size_t ix = 0; ix < 10; ++ix) {
    pq.increase(ix);
    int val = pq[ix];
    pq.increase(ix, val);
    *(pq.begin() + ix) = val;
    pq.increase(ix);
    EXPECT_TRUE(pq.is_valid());
  }
  ss2 << pq;
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, UpdateNoop) {
  std::stringstream ss1, ss2;
  ss1 << pq;
  for (size_t ix = 0; ix < 10; ++ix) {
    pq.update(ix);
    int val = pq[ix];
    pq.update(ix, val);
    *(pq.begin() + ix) = val;
    pq.update(ix);
    EXPECT_TRUE(pq.is_valid());
  }
  ss2 << pq;
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, Decrease3) {
  Priority_queue<int> pqcopy = pq;
  const int old_priority = pq[3];
  const int new_priority = old_priority - rand() / 2;

  pq.decrease(3, new_priority);
  pqcopy[3] = new_priority;
  pqcopy.decrease(3);

  std::stringstream ss1, ss2;
  ss1 << pq;
  ss2 << pqcopy;
  EXPECT_TRUE(pq.is_valid());
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, Increase4) {
  Priority_queue<int> pqcopy = pq;
  const int old_priority = pq[4];
  const int new_priority = old_priority + rand() / 2;

  pq.increase(4, new_priority);
  pqcopy[4] = new_priority;
  pqcopy.increase(4);

  std::stringstream ss1, ss2;
  ss1 << pq;
  ss2 << pqcopy;
  EXPECT_TRUE(pq.is_valid());
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, Update2) {
  Priority_queue<int> pqcopy = pq;

  for (int i = -10; i <= 10; ++i) {
    const int old_priority = pq[2];
    const int new_priority = old_priority + i;

    pq.update(2, new_priority);
    pqcopy[2] = new_priority;
    pqcopy.update(2);

    std::stringstream ss1, ss2;
    ss1 << pq;
    ss2 << pqcopy;
    EXPECT_TRUE(pq.is_valid()) << "i:" << i << " pq:" << pq;
    EXPECT_TRUE(pqcopy.is_valid());
    EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
  }
}

TEST_F(PriorityQueueTest, UpdateTop) {
  Priority_queue<int> pqcopy = pq;
  const int old_priority = pq.top();
  const int new_priority = old_priority + 10;

  pq.top() = new_priority;
  pq.update_top();
  pqcopy.update(0, new_priority);
  std::stringstream ss1, ss2;
  ss1 << pq;
  ss2 << pqcopy;
  EXPECT_TRUE(pq.is_valid());
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, PopAndRemove) {
  Priority_queue<int> pqcopy = pq;

  EXPECT_TRUE(pqcopy.size() == 10U);
  pqcopy.pop();
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_TRUE(pqcopy.size() == 9U);
  pqcopy.remove(3);
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_TRUE(pqcopy.size() == 8U);
  pqcopy.remove(pqcopy.size() - 1);
  EXPECT_TRUE(pqcopy.is_valid());
  EXPECT_TRUE(pqcopy.size() == 7U);

  Priority_queue<int> singleton;
  EXPECT_FALSE(singleton.push(10));
  singleton.pop();
  EXPECT_TRUE(singleton.empty());
  EXPECT_TRUE(singleton.is_valid());
}

TEST_F(PriorityQueueTest, Iterators) {
  Priority_queue<int>::iterator it;
  Priority_queue<int>::const_iterator cit;
  std::stringstream ss1, ss2;
  for (it = pq.begin(); it != pq.end(); ++it) ss1 << *it << " ";
  for (cit = pq.begin(); cit != pq.end(); ++cit) ss2 << *cit << " ";
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, Clear) {
  EXPECT_EQ(10U, pq.size());
  EXPECT_TRUE(pq.is_valid());
  pq.clear();
  EXPECT_EQ(0U, pq.size());
  EXPECT_TRUE(pq.is_valid());
}

TEST_F(PriorityQueueTest, Reserve) {
  EXPECT_EQ(10U, pq.capacity());
  EXPECT_EQ(10U, pq.size());
  EXPECT_FALSE(pq.reserve(10));
  EXPECT_EQ(10U, pq.capacity());
  EXPECT_EQ(10U, pq.size());
  EXPECT_FALSE(pq.reserve(5));
  EXPECT_EQ(10U, pq.capacity());
  EXPECT_EQ(10U, pq.size());
  EXPECT_FALSE(pq.reserve(20));
  EXPECT_EQ(20U, pq.capacity());
  EXPECT_EQ(10U, pq.size());
}

TEST_F(PriorityQueueTest, Sort) {
  Priority_queue<int> pqcopy = pq;
  std::vector<int> keyscopy(keys, keys + 10);

  pqcopy.sort();
  std::sort(keyscopy.begin(), keyscopy.end());

  std::stringstream ss1, ss2;
  ss1 << pqcopy;
  for (size_t i = 0; i < keyscopy.size(); ++i) {
    ss2 << keyscopy[i] << " ";
  }
  EXPECT_STREQ(ss1.str().c_str(), ss2.str().c_str());
}

TEST_F(PriorityQueueTest, TestHeapKeys) {
  SCOPED_TRACE("");
  test_heap(keys, keys + 10);
}

TEST_F(PriorityQueueTest, TestHeapKeys2) {
  SCOPED_TRACE("");
  test_heap(keys2.begin(), keys2.end());
}

TEST_F(PriorityQueueTest, TestHeapOfHandles) {
  SCOPED_TRACE("");
  test_heap_of_handles(keys, keys + 10);
}

TEST_F(PriorityQueueTest, TestHeapOfHandles2) {
  SCOPED_TRACE("");
  test_heap_of_handles(keys2.begin(), keys2.end());
}

TEST_F(PriorityQueueTest, TestMinKElements) {
  SCOPED_TRACE("");
  test_min_k_elements(keys, keys + 10, 7);
}

TEST_F(PriorityQueueTest, TestMinKElements2) {
  SCOPED_TRACE("");
  test_min_k_elements(keys2.begin(), keys2.end(), 4);
}

TEST_F(PriorityQueueTest, RandomIntegerGenerator) {
  random_integer_generator<int> g;
  std::vector<int> many_keys;

  for (int i = 0; i < 200; ++i) {
    int value = g(0, 300);
    many_keys.push_back(value);
  }

  SCOPED_TRACE("");
  test_min_k_elements(many_keys.begin(), many_keys.end(), 20);
}

/**
  Bug#30301356 - SOME EVENTS ARE DELAYED AFTER DROPPING EVENT

  Test that ensures heap property is not violated if we remove an
  element from an interior node. In the below test, we remove the
  element 90 at index 6 in the array. After 90 is removed, the
  parent node's of the deleted node violates the heap property.
  In order to restore the heap property, we need to move up the
  heap until we reach a node which satisfies the heap property or
  the root. Without the fix, we adjust the heap downwards.
*/

TEST_F(PriorityQueueTest, TestElementRemove) {
  Priority_queue<int, std::vector<int>, My_greater> pq;

  int keys[11] = {60, 65, 84, 75, 80, 85, 90, 95, 100, 105, 82};
  pq = Priority_queue<int, std::vector<int>, My_greater>(keys, keys + 11);
  pq.remove(6);
  EXPECT_TRUE(pq.is_valid());
}
}  // namespace priority_queue_unittest
