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

#include "hphp/runtime/vm/record.h"

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/treadmill.h"

namespace HPHP {

Mutex g_recordsMutex;

  RecordDesc::RecordDesc(Unit* unit, int line1, int line2,
                         const StringData* n, Attr attrs,
                         const StringData* docComment, Id id)
    : m_unit(unit)
    , m_namedEntity(NamedEntity::get(n))
    , m_line1(line1)
    , m_line2(line2)
    , m_id(id)
    , m_attrs(attrs)
    , m_name(n)
    , m_docComment(docComment) {}

  void RecordDesc::atomicRelease() {
    assertx(!m_cachedRecordDesc.bound());
    assertx(!getCount());
    this->~RecordDesc();
  }

  void RecordDesc::setCached() {
    m_cachedRecordDesc.initWith(this);
  }

  void RecordDesc::setRecordDescHandle(
      rds::Link<LowPtr<RecordDesc>, rds::Mode::NonLocal> link) const {
    assertx(!m_cachedRecordDesc.bound());
    m_cachedRecordDesc = link;
  }

  void RecordDesc::destroy() {
    if (!m_cachedRecordDesc.bound()) return;

    Lock l(g_recordsMutex);
    if (!m_cachedRecordDesc.bound()) return;
    m_cachedRecordDesc = rds::Link<LowPtr<RecordDesc>, rds::Mode::NonLocal>{};
    namedEntity()->removeRecordDesc(this);

    Treadmill::enqueue(
      [this] {
        if (!this->decAtomicCount()) this->atomicRelease();
      }
    );
  }

  RecordDesc::Field::Field(RecordDesc* record,
                           const StringData* name,
                           Attr attrs,
                           const StringData* userType,
                           const TypeConstraint& typeConstraint,
                           const StringData* docComment,
                           const TypedValue& val,
                           RepoAuthType repoAuthType,
                           UserAttributeMap userAttributes)
  : m_name(name)
  , m_mangledName(mangleFieldName(record->name(), name, attrs))
  , m_attrs(attrs)
  , m_userType{userType}
  , m_docComment(docComment)
  , m_val(val)
  , m_repoAuthType{repoAuthType}
  , m_typeConstraint{typeConstraint}
  , m_userAttributes(userAttributes)
{}

const StringData* RecordDesc::mangleFieldName(
    const StringData* recordName,
    const StringData* fieldName,
    Attr attrs) {
  return PreClass::manglePropName(recordName, fieldName, attrs);
}

Slot RecordDesc::lookupField(const StringData* fieldName) const {
  auto idx = m_fields.findIndex(fieldName);
  if (idx == kInvalidSlot) {
    raise_error(folly::sformat("Field '{}' does not exist in record '{}'",
                               fieldName, name()));
  }
  return idx;
}

const RecordDesc::Field& RecordDesc::field(const StringData* fieldName) const {
  return m_fields[lookupField(fieldName)];
}

void RecordDesc::checkFieldDefaultValues() const {
  for (auto const& field : allFields()) {
    auto const& val = field.val();
    if (val.m_type == KindOfUninit) continue;
    auto const& tc = field.typeConstraint();
    if (tc.isCheckable()) tc.verifyRecField(&val, m_name, field.name());
  }
}
}
