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

#ifndef incl_HPHP_RECORD_DATA_INL_H_
#error "record-data-inl.h should only be included by record-data.h"
#endif

#include "hphp/runtime/vm/record.h"

namespace HPHP {

inline size_t RecordData::heapSize() const {
  return sizeWithFields(m_record);
}

inline const RecordDesc* RecordBase::record() const {
  return m_record;
}

inline bool RecordData::kindIsValid() const {
  return m_kind == HeaderKind::Record;
}

inline const TypedValue* RecordBase::fieldVec() const {
  return reinterpret_cast<const TypedValue*>(uintptr_t(this + 1));
}

}
