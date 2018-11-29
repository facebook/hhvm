/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_JIT_VASM_DATA_H_
#define incl_HPHP_JIT_VASM_DATA_H_

#include "hphp/runtime/vm/jit/containers.h"

#include "hphp/util/assertions.h"

#include <cstdint>
#include <memory>
#include <string>

namespace HPHP { namespace jit {

/*
 * VdataPtr represents a pointer to a value that will end up in the JIT's
 * global data section.
 *
 * When bound() == false, it is a pointer to somewhere on the heap. When
 * bound() == true, it points to the data's final resting place in the data
 * section. The data being pointed to must be a POD type with no internal
 * pointers: it will be relocated using std::memcpy() and never destroyed.
 */
template<typename T>
struct VdataPtr {
  enum class Bound {};

  /* implicit */ VdataPtr(T* ptr) : m_ptr(ptr), m_bound(false) {}

  VdataPtr(const VdataPtr& other) = default;
  VdataPtr& operator=(const VdataPtr& other) = default;

  bool operator==(const VdataPtr& o) const {
    return m_ptr == o.m_ptr && m_bound == o.m_bound;
  }
  bool operator!=(const VdataPtr& o) const {
    return m_ptr != o.m_ptr || m_bound != o.m_bound;
  }

  bool bound() const {
    return m_bound;
  }

  T* get() const {
    assertx(m_bound);
    return m_ptr;
  }

  T* getRaw() const {
    return m_ptr;
  }

  void bind(T* ptr) {
    m_ptr = ptr;
    m_bound = true;
  }

private:
  T* m_ptr;
  bool m_bound;
};

/*
 * A block of data that will be moved into the JIT's global data section right
 * before final code emission.
 *
 * VdataPtrs may point anywhere in the buffer owned by a VdataBlock, and they
 * will be fixed up appropriately.
 */
struct VdataBlock {
  jit::unique_ptr<uint8_t[]> data;
  size_t size;
  size_t align;
};

}}

#endif
