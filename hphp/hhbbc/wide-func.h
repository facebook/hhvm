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
#pragma once

#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC { namespace php {

//////////////////////////////////////////////////////////////////////

/*
 * Run a correctness test of compression: compress and decompress all the
 * Funcs in the given Program. Although we take a mutable ref to the
 * program as input, this operation shouldn't change the program's code.
 */
void testCompression(Program& program);

/*
 * Only used to implement WideFunc::blockRange. Probably overkill.
 */
template <typename T>
struct IntLikeIterator {
  explicit IntLikeIterator(T v) : val{v} {}
  T operator *() const { return val; }
  T operator ++() { return ++val; }
  bool operator !=(IntLikeIterator other) const { return val != other.val; }
private:
  T val;
};
template <typename T>
struct IntLikeRange {
  template<typename C>
  explicit IntLikeRange(const C& v) : max(v.size()) {}
  IntLikeIterator<T> begin() const { return IntLikeIterator<T>{0}; }
  IntLikeIterator<T> end() const { return IntLikeIterator<T>{max}; }
private:
  T max;
};

/*
 * We keep the code of a Func compressed at rest, so you must instantiate
 * a "wide pointer" to read or write its block data. These pointers store
 * a copy of the expanded data, so instantiating them is a heavy-weight
 * operation and they cannot be copied or moved.
 */
struct WideFunc {
  WideFunc(WideFunc&&) = delete;
  WideFunc(const WideFunc&) = delete;
  WideFunc& operator=(WideFunc&&) = delete;
  WideFunc& operator=(const WideFunc&) = delete;

  static WideFunc mut(Func* func) { return WideFunc(func, true); }
  static const WideFunc cns(const Func* func) { return WideFunc(func, false); }

  ~WideFunc();

  operator Func*() { return m_func; }
  Func& operator*() { return *m_func; }
  Func* operator->() { return m_func; }
  BlockVec& blocks() { return m_blocks; }

  operator const Func*() const { return m_func; }
  const Func& operator*() const { return *m_func; }
  const Func* operator->() const { return m_func; }
  const BlockVec& blocks() const { return m_blocks; }

  operator bool() const { return m_func; }
  auto blockRange() const { return IntLikeRange<BlockId>{m_blocks}; }

  // Call release if the backing Func is destroyed while this WideFunc is live.
  void release();

private:
  WideFunc(const Func* func, bool mut);

  Func* m_func;
  BlockVec m_blocks;
  bool m_mut;
};

//////////////////////////////////////////////////////////////////////

}}}

