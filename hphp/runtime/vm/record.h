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

#ifndef incl_HPHP_VM_RECORD_H_
#define incl_HPHP_VM_RECORD_H_

#include "hphp/runtime/base/atomic-countable.h"
#include "hphp/runtime/base/atomic-shared-ptr.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/vm/indexed-string-map.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/lock.h"

#include <type_traits>
#include <unordered_set>

namespace HPHP {

struct RecordEmitter;

struct Record : AtomicCountable {
  friend struct RecordEmitter;

  struct Field {
    Field(Record* record,
          const StringData* name,
          Attr attrs,
          const StringData* userType,
          const TypeConstraint& typeConstraint,
          const StringData* docComment,
          const TypedValue& val,
          RepoAuthType repoAuthType,
          UserAttributeMap userAttributes);

    const StringData* name() const { return m_name; }
    const StringData* mangledName() const { return m_mangledName; }
    Attr attrs() const { return m_attrs; }
    const StringData* userType() const { return m_userType; }
    const TypeConstraint& typeConstraint() const { return m_typeConstraint; }
    const StringData* docComment() const { return m_docComment; }
    const TypedValue& val() const { return m_val; }
    RepoAuthType repoAuthType() const { return m_repoAuthType; }
    const UserAttributeMap& userAttributes() const { return m_userAttributes; }

  private:
    LowStringPtr m_name;
    LowStringPtr m_mangledName;
    Attr m_attrs;
    LowStringPtr m_userType;
    LowStringPtr m_docComment;
    TypedValue m_val;
    RepoAuthType m_repoAuthType;
    TypeConstraint m_typeConstraint;
    UserAttributeMap m_userAttributes;
  };

  /*
   * Called when the (atomic) refcount hits zero.
   *
   * The Record is completely dead at this point, and its memory is freed
   * immediately.
   */
  void atomicRelease();

private:
  typedef IndexedStringMap<Field,true,Slot> FieldMap;

public:
  Record(Unit* unit, int line1, int line2, const StringData* n,
         Attr attrs, const StringData* docComment, Id id);


  Unit*             unit()         const { return m_unit; }
  NamedEntity*      namedEntity()  const { return m_namedEntity; }
  int               line1()        const { return m_line1; }
  int               line2()        const { return m_line2; }
  Id                id()           const { return m_id; }
  Attr              attrs()        const { return m_attrs; }
  const StringData* name()         const { return m_name; }
  const StringData* docComment()   const { return m_docComment; }

  static const StringData* mangleFieldName(const StringData* recordName,
                                           const StringData* fieldName,
                                           Attr attrs);

  size_t numFields() const { return m_fields.size(); }
  using FieldRange = folly::Range<const Field*>;
  FieldRange allFields() const {
    return FieldRange(m_fields.accessList(), m_fields.size());
  }
  Slot lookupField(const StringData*) const;

  AtomicLowPtr<Record> m_next{nullptr}; // used by NamedEntity

  void setCached();
  void setRecordHandle(rds::Link<LowPtr<Record>,
                                 rds::Mode::NonLocal> link) const;

  void destroy();

private:
  Unit* m_unit;
  LowPtr<NamedEntity> m_namedEntity;
  int m_line1;
  int m_line2;
  Id m_id;
  Attr m_attrs;
  LowStringPtr m_name;
  LowStringPtr m_docComment;
  FieldMap m_fields;

  mutable rds::Link<LowPtr<Record>, rds::Mode::NonLocal> m_cachedRecord;
};

extern Mutex g_recordsMutex;

typedef AtomicSharedPtr<Record> RecordPtr;

///////////////////////////////////////////////////////////////////////////////
}

#endif
