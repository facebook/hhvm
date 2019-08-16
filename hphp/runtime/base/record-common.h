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

#ifndef incl_HPHP_RECORD_COMMON_H_
#define incl_HPHP_RECORD_COMMON_H_

#include "hphp/runtime/vm/record.h"

namespace HPHP {
struct StringData;

struct RecordBase {
  const RecordDesc* record() const;

  tv_rval fieldRval(const StringData*) const;
  tv_lval fieldLval(const StringData*);

  static size_t fieldSize(const RecordDesc* rec) {
    return sizeof(TypedValue) * rec->numFields();
  }

protected:
  explicit RecordBase(const RecordDesc*);
  const TypedValue* fieldVec() const;
  void scan(type_scan::Scanner&) const;
  const RecordDesc* const m_record;

  template<class RecordType>
  static RecordType* newRecordImpl(const RecordDesc*,
                                   uint32_t initSize,
                                   const StringData* const* keys,
                                   const TypedValue* values);
};
} // namespace HPHP
#endif // incl_HPHP_RECORD_COMMON_H_
