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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"


namespace HPHP {

template <class T, class I>
struct NonNullReturnType {
  // these functions is needed only for recorder-replayer's sake
  I detach_do_not_use() {
    auto ret = I::attach(m_data);
    m_data = nullptr;
    return ret;
  }
  T* get_do_not_use() const {
    return m_data;
  }

  // We don't really want a default constructor
  // but are forced to declare one because HHVM record/replay would break otherwise
  NonNullReturnType() = default;

  /*implicit*/ NonNullReturnType(I&& data) : m_data(data.detach()) {
    assertx(m_data != nullptr);
  }
  /*implicit*/ NonNullReturnType(const I&& data) = delete;

  static NonNullReturnType<T, I> attach(T* data) {
    return NonNullReturnType(I::attach(data));
  }
private:
  T* m_data;
};

} // namespace HPHP

using ArrayRet = HPHP::NonNullReturnType<HPHP::ArrayData, HPHP::Array>;
using StringRet = HPHP::NonNullReturnType<HPHP::StringData, HPHP::String>;
using ObjectRet = HPHP::NonNullReturnType<HPHP::ObjectData, HPHP::Object>;
using ResourceRet = HPHP::NonNullReturnType<HPHP::ResourceData, HPHP::OptResource>;
