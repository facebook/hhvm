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

#include <gtest/gtest.h>
#include <stddef.h>
#include <algorithm>
#include <deque>
#include <list>
#include <memory>
#include <vector>

#include "my_inttypes.h"
#include "sql/malloc_allocator.h"
#include "sql/mem_root_allocator.h"
#include "sql/psi_memory_key.h"
#include "sql/stateless_allocator.h"
#include "sql/thr_malloc.h"

using std::deque;
using std::list;
using std::vector;

/*
  Tests of custom STL memory allocators.
*/

#if defined(GTEST_HAS_TYPED_TEST)

namespace stlalloc_unittest {

/*
  Wrappers to overcome the issue that we need allocators with
  default constructors for TYPED_TEST_CASE, which neither
  Malloc_allocator nor Mem_root_allocator have.

  These wrappers need to inherit so that they are allocators themselves.
  Otherwise TypeParam in the tests below will be wrong.
*/
template <typename T>
class Malloc_allocator_wrapper : public Malloc_allocator<T> {
 public:
  Malloc_allocator_wrapper() : Malloc_allocator<T>(PSI_NOT_INSTRUMENTED) {}
};

template <typename T>
class Mem_root_allocator_wrapper : public Mem_root_allocator<T> {
  MEM_ROOT m_mem_root;

 public:
  Mem_root_allocator_wrapper() : Mem_root_allocator<T>(&m_mem_root) {
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_mem_root, 1024, 0);
    // memory allocation error is expected, don't abort unit test.
    m_mem_root.set_error_handler(nullptr);
  }

  /*
    Allocators before C++17 need to be copy-constructible, and libc++ enforces
    this (libstdc++ is fine with them being move-only). As a hack, we implement
    copying the MEM_ROOT here; this type is never used outside of unit tests.

    Note that this will stop working if MEM_ROOT grows a destructor.
  */
  Mem_root_allocator_wrapper(const Mem_root_allocator_wrapper &other)
      : Mem_root_allocator<T>(&m_mem_root) {
    memcpy(&m_mem_root, &other.m_mem_root, sizeof(m_mem_root));
  }

  ~Mem_root_allocator_wrapper() { free_root(&m_mem_root, MYF(0)); }
};

/*
  Flag for turning on allocation failure simulation.
*/
static bool simulate_failed_allocation = false;

/*
  Custom std::unique_ptr "Deleter" to reset flag. Allows unique_ptr to be used
  as scope guard which ensures that the flag is reset.
*/
static auto reset_sfa = [](bool *sfap) { *sfap = false; };

/*
  Local utility function which calls my_malloc with the standard MYF flags.
*/

static void *my_malloc_(PSI_memory_key k, size_t s) {
  return simulate_failed_allocation
             ? nullptr
             : my_malloc(k, s, MYF(MY_WME | ME_FATALERROR));
}

/* Functor for un-instrumented my_malloc */
struct Not_instr_alloc {
  void *operator()(size_t s) const {
    return my_malloc_(PSI_NOT_INSTRUMENTED, s);
  }
};

/*
   Template alias for a Stateless_allocator using un-instrumented
   my_malloc allocation. Deallocation functor is the default template
   argument which is functor invoking my_free.
*/
template <class T>
using Not_instr_allocator = Stateless_allocator<T, Not_instr_alloc>;

/*
  Templated functor for my_malloc allocation using a PSI_KEY specified
  a compile time. This is generally not that useful outside unit
  testing as the PSI_KEY value is not normally known at compile time.
*/
template <int PSI_KEY>
struct PSI_key_alloc {
  void *operator()(size_t s) const { return my_malloc_(PSI_KEY, s); }
};

/*
  Template alias for a Stateless allocator which allocates with
  my_malloc and PSI key 42.
*/
template <class T>
using PSI_42_allocator = Stateless_allocator<T, PSI_key_alloc<42>>;

/*
  Functor which allocates using the global operator new and
  initializes the allocated memory with the value provided in the
  template argument.
 */
template <unsigned char INIT>
struct Init_alloc {
  void *operator()(size_t s) const {
    if (simulate_failed_allocation ||
        DBUG_EVALUATE_IF("simulate_out_of_memory", true, false)) {
      return nullptr;
    }

    char *buf = static_cast<char *>(operator new(s));
    memset(buf, INIT, s);
    return buf;
  }
};

/*
  Functor which deallocates using the global operator delete and and
  writes the value provided in the template argument into the memory
  being released.
 */
template <unsigned char TRASH>
struct Trash_dealloc {
  void operator()(void *p, size_t s) const {
    memset(p, TRASH, s);
    operator delete(p);
  }
};

/*
  Template alias for a Stateless_allocator using initialized
  allocation with new and trash-filled deallocation with delete
*/
template <class T>
using Init_aa_allocator =
    Stateless_allocator<T, Init_alloc<0xaa>, Trash_dealloc<0xbb>>;

//
// Test of container with simple objects
//

template <typename T>
class STLAllocTestInt : public ::testing::Test {
 protected:
  T allocator;
};

typedef ::testing::Types<
    Malloc_allocator_wrapper<int>, Mem_root_allocator_wrapper<int>,
    Not_instr_allocator<int>, PSI_42_allocator<int>, Init_aa_allocator<int>>
    AllocatorTypesInt;

TYPED_TEST_CASE(STLAllocTestInt, AllocatorTypesInt);

TYPED_TEST(STLAllocTestInt, SimpleVector) {
  vector<int, TypeParam> v1(this->allocator);
  vector<int, TypeParam> v2(this->allocator);
  for (int i = 0; i < 100; i++) {
    v1.push_back(i);
    v2.push_back(100 - i);
  }

  EXPECT_EQ(100U, v1.size());
  EXPECT_EQ(100U, v2.size());

  v1.swap(v2);

  EXPECT_EQ(100U, v1.size());
  EXPECT_EQ(100U, v2.size());

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(i, v2[i]);
    EXPECT_EQ((100 - i), v1[i]);
  }
}

TYPED_TEST(STLAllocTestInt, SimpleList) {
  list<int, TypeParam> l1(this->allocator);
  list<int, TypeParam> l2(this->allocator);

  for (int i = 0; i < 100; i++) l1.push_back(i);

  EXPECT_EQ(100U, l1.size());
  EXPECT_EQ(0U, l2.size());

  l2.splice(l2.begin(), l1);

  EXPECT_EQ(0U, l1.size());
  EXPECT_EQ(100U, l2.size());

  std::reverse(l2.begin(), l2.end());

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ((99 - i), l2.front());
    l2.pop_front();
  }

  EXPECT_EQ(0U, l2.size());
}

#ifndef DBUG_OFF
TYPED_TEST(STLAllocTestInt, OutOfMemory) {
  vector<int, TypeParam> v1(this->allocator);
  v1.reserve(10);
  EXPECT_EQ(10U, v1.capacity());

  DBUG_SET("+d,simulate_out_of_memory");
  ASSERT_THROW(v1.reserve(1000), std::bad_alloc);
  DBUG_SET("-d,simulate_out_of_memory");
}
#endif

//
// Test of container with non-trival objects
//

class Container_object;

template <typename T>
class STLAllocTestObject : public STLAllocTestInt<T> {};

typedef ::testing::Types<Malloc_allocator_wrapper<Container_object>,
                         Mem_root_allocator_wrapper<Container_object>,
                         Not_instr_allocator<Container_object>,
                         PSI_42_allocator<Container_object>,
                         Init_aa_allocator<Container_object>>
    AllocatorTypesObject;

TYPED_TEST_CASE(STLAllocTestObject, AllocatorTypesObject);

class Container_object {
  char *buffer;

 public:
  Container_object() { buffer = new char[20]; }

  Container_object(const Container_object &) {
    buffer = new char[20];  // Don't care about contents
  }

  ~Container_object() { delete[] buffer; }
};

TYPED_TEST(STLAllocTestObject, ContainerObject) {
  vector<Container_object, TypeParam> v1(this->allocator);
  v1.push_back(Container_object());
  v1.push_back(Container_object());
  v1.push_back(Container_object());
}

//
// Test of container with containers
//

class Container_container;

template <typename T>
class STLAllocTestNested : public STLAllocTestInt<T> {};

typedef ::testing::Types<Malloc_allocator_wrapper<Container_container>,
                         Mem_root_allocator_wrapper<Container_container>,
                         Not_instr_allocator<Container_container>,
                         PSI_42_allocator<Container_container>,
                         Init_aa_allocator<Container_container>>
    AllocatorTypesNested;

TYPED_TEST_CASE(STLAllocTestNested, AllocatorTypesNested);

class Container_container {
  deque<Container_object> d;

 public:
  Container_container() {
    d.push_back(Container_object());
    d.push_back(Container_object());
  }
};

TYPED_TEST(STLAllocTestNested, NestedContainers) {
  Container_container cc1;
  Container_container cc2;
  list<Container_container, TypeParam> l1(this->allocator);
  l1.push_back(cc1);
  l1.push_back(cc2);
}

//
// Test that it is possible to instantiate the std::basic_string
// template with various Stateless_allocator instances.
//

/*
  Template alias for basic_string with char.
 */
template <class A>
using default_string = std::basic_string<char, std::char_traits<char>, A>;

template <class A>
class STLAllocTestBasicStringTemplate : public ::testing::Test {};

/*
  Cannot use the stateful allocators with basic_string. The following
  will not compile, due to
  https://bugzilla.redhat.com/show_bug.cgi?id=1546704
  "Define _GLIBCXX_USE_CXX11_ABI gets ignored by gcc in devtoolset-7"

   typedef std::basic_string<char, std::char_traits<char>,
                             Malloc_allocator<char> > MA_string_type;
   MA_string_type y("bar", Malloc_allocator<char>(42));
*/
typedef ::testing::Types<Not_instr_allocator<char>, PSI_42_allocator<char>,
                         Init_aa_allocator<char>>
    AllocatorTypesBasicStringTemplate;

TYPED_TEST_CASE(STLAllocTestBasicStringTemplate,
                AllocatorTypesBasicStringTemplate);

//
// Verify that a default_string can be created and extended with the
// Stateless_allocator instantiations.
//
TYPED_TEST(STLAllocTestBasicStringTemplate, BasicTest) {
  typedef default_string<TypeParam> String_type;

  String_type x("foobar");
  x += "_tag";
  EXPECT_EQ(10U, x.size());
}

//
// Verify that std::bad_alloc is thrown in out-of-memory conditions
//
TYPED_TEST(STLAllocTestBasicStringTemplate, OutOfMemTest) {
  // Scope guard which ensures flag is reset
  std::unique_ptr<bool, decltype(reset_sfa)> sg(&simulate_failed_allocation,
                                                reset_sfa);

  typedef default_string<TypeParam> String_type;

  String_type x("foobar");

  // Set flag to force allocation failure
  simulate_failed_allocation = true;
  ASSERT_THROW(x.reserve(1000), std::bad_alloc);
}

//
// Test of container of objects that cannot be copied.
//

template <typename T>
class STLAllocTestMoveOnly : public STLAllocTestInt<T> {};

typedef ::testing::Types<Malloc_allocator_wrapper<std::unique_ptr<int>>,
                         Mem_root_allocator_wrapper<std::unique_ptr<int>>,
                         Not_instr_allocator<std::unique_ptr<int>>,
                         PSI_42_allocator<std::unique_ptr<int>>,
                         Init_aa_allocator<std::unique_ptr<int>>>
    AllocatorTypesMoveOnly;

TYPED_TEST_CASE(STLAllocTestMoveOnly, AllocatorTypesMoveOnly);

TYPED_TEST(STLAllocTestMoveOnly, MoveOnly) {
  vector<std::unique_ptr<int>, TypeParam> v(this->allocator);
  v.emplace_back(new int(1));
  v.emplace_back(new int(2));
  EXPECT_EQ(1, *v[0]);
  EXPECT_EQ(2, *v[1]);
}

}  // namespace stlalloc_unittest

#endif  // GTEST_HAS_TYPED_TEST)
