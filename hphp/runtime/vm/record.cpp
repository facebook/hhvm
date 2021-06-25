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
#include "hphp/runtime/vm/unit.h"

namespace HPHP {

Mutex g_recordsMutex;

PreRecordDesc::PreRecordDesc(Unit* unit, int line1, int line2,
                             const StringData* name, Attr attrs,
                             const StringData* parentName,
                             const StringData* docComment, Id id)
  : m_unit(unit)
  , m_namedEntity(NamedEntity::get(name))
  , m_line1(line1)
  , m_line2(line2)
  , m_id(id)
  , m_attrs(attrs)
  , m_name(name)
  , m_parentName(parentName)
  , m_docComment(docComment) {}

void PreRecordDesc::atomicRelease() {
  delete this;
}

PreRecordDesc::Field::Field(PreRecordDesc* record,
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

const StringData* PreRecordDesc::mangleFieldName(
    const StringData* recordName,
    const StringData* fieldName,
    Attr attrs) {
  return PreClass::manglePropName(recordName, fieldName, attrs);
}

void PreRecordDesc::checkDefaultValueType(
    const PreRecordDesc::Field& field) const {
  auto const& val = field.val();
  if (val.m_type == KindOfUninit) return;
  auto const& tc = field.typeConstraint();
  if (tc.isCheckable()) tc.verifyRecField(&val, m_name, field.name());
}


size_t RecordDesc::stableHash() const {
  return folly::hash::hash_combine(
    name()->hashStatic(),
    unit()->sn()
  );
}

void RecordDesc::atomicRelease() {
  assertx(!m_cachedRecordDesc.bound());
  assertx(!getCount());
  this->~RecordDesc();
  lower_free(this);
}

void RecordDesc::setCached() {
  m_cachedRecordDesc.initWith(this);
}

void RecordDesc::setRecordDescHandle(
    rds::Link<LowPtr<RecordDesc>, rds::Mode::NonLocal> link) const {
  assertx(!m_cachedRecordDesc.bound());
  m_cachedRecordDesc = link;
}

bool RecordDesc::verifyPersistent() const {
  if (!isPersistent()) return false;
  assertx(!m_parent || recordHasPersistentRDS(m_parent.get()));
  return true;
}

void RecordDesc::destroy() {
  if (!m_cachedRecordDesc.bound()) return;

  Lock l(g_recordsMutex);
  if (!m_cachedRecordDesc.bound()) return;
  m_cachedRecordDesc = rds::Link<LowPtr<RecordDesc>, rds::Mode::NonLocal>{};
  m_preRec->namedEntity()->removeRecordDesc(this);
  m_parent.reset();

  Treadmill::enqueue(
    [this] {
      if (!this->decAtomicCount()) this->atomicRelease();
    }
  );
}

Slot RecordDesc::lookupField(const StringData* fieldName) const {
  return m_fields.findIndex(fieldName);
}

const RecordDesc::Field& RecordDesc::field(Slot idx) const {
  assertx(idx != kInvalidSlot && idx < numFields());
  return m_fields[idx];
}

RecordDesc::Avail RecordDesc::availWithParent(
    RecordDesc*& parent, bool tryAutoload /* = false */) const {
  if (RecordDesc* ourParent = m_parent.get()) {
    if (!parent) {
      auto pprec = ourParent->m_preRec.get();
      parent = Unit::getRecordDesc(pprec->namedEntity(),
                                   parentName(), tryAutoload);
      if (!parent) {
        parent = ourParent;
        return Avail::Fail;
      }
    }
    if (parent != ourParent) {
      if (UNLIKELY(ourParent->isZombie())) {
        const_cast<RecordDesc*>(this)->destroy();
      }
      return Avail::False;
    }
  }
  return Avail::True;
}

RecordDesc* RecordDesc::newRecordDesc(PreRecordDesc* preRec,
                                      RecordDesc* parent) {
  auto const mem = lower_malloc(sizeof(RecordDesc));
  try {
    return new (mem) RecordDesc(preRec, parent);
  } catch (...) {
    lower_free(mem);
    throw;
  }
}

void RecordDesc::setParent() {
  // validate the parent
  if (m_parent.get() != nullptr) {
    auto parentAttrs = m_parent->attrs();
    if (UNLIKELY(parentAttrs & AttrFinal)) {
      raise_error("Record %s may not inherit from non-abstract record %s",
                  name()->data(), m_parent->name()->data());
    }
  }
}

void RecordDesc::setFields() {
  FieldMap::Builder curFieldMap;
  if (m_parent.get() != nullptr) {
    for (auto const& parentField : m_parent->allFields()) {
      curFieldMap.add(parentField.name(), parentField);
    }
  }
  for (auto const& preField : m_preRec->allFields()) {
    auto const it = curFieldMap.find(preField.name());
    if (it != curFieldMap.end()) {
      raise_error("Cannot redeclare record field %s::%s",
                  name()->data(),
                  preField.name()->data());
    }
    m_preRec->checkDefaultValueType(preField);
    curFieldMap.add(preField.name(), preField);
  }
  m_fields.create(curFieldMap);
}

RecordDesc::RecordDesc(PreRecordDesc* preRec, RecordDesc* parent)
  : m_parent(parent)
  , m_preRec(PreRecordDescPtr(preRec)) {
  setParent();
  setFields();
}

bool RecordDesc::recordDescOf(const RecordDesc* rec) const {
  // TODO: Optimize this. See T45403957.
  assertx(rec);
  auto curr = this;
  while(curr) {
    if (curr == rec) return true;
    curr = curr->m_parent.get();
  }
  return false;
}

const RecordDesc* RecordDesc::commonAncestor(const RecordDesc* rec) const {
  // TODO: Optimize this. See T45403957.
  assertx(rec);
  std::vector<const RecordDesc*> thisVec;
  std::vector<const RecordDesc*> recVec;
  for (auto r = this; r != nullptr; r = r->m_parent.get()) thisVec.push_back(r);
  for (auto r = rec; r != nullptr; r = r->m_parent.get()) recVec.push_back(r);
  auto idx = std::min(thisVec.size(), recVec.size()) - 1;
  do {
    if (thisVec[idx] == recVec[idx]) return thisVec[idx];
  } while(idx--);
  return nullptr;
}

}
