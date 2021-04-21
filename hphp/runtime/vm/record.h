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

/*
 * A PreRecordDesc represents the source-level definition of a Hack Record.
 * Includes name of the parent class (if any) and metadata about fields
 * declared in the record.
 *
 * This is separate for an actual RecordDesc which represents a request specific
 * instantiation of a record type. For example, the parent record name may
 * resolve to different RecordDescs in different requests.
 */
struct PreRecordDesc : AtomicCountable {
  friend struct RecordEmitter;
  friend struct RecordDesc;

  struct Field {
    Field(PreRecordDesc* record,
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
   * The PreRecordDesc is completely dead at this point, and its memory is freed
   * immediately.
   */
  void atomicRelease();

private:
  using FieldMap = IndexedStringMap<Field,Slot>;
  void checkDefaultValueType(const Field&) const;

public:
  PreRecordDesc(Unit* unit, int line1, int line2, const StringData* name,
                Attr attrs, const StringData* parentName,
                const StringData* docComment, Id id);


  Unit*             unit()         const { return m_unit; }
  NamedEntity*      namedEntity()  const { return m_namedEntity; }
  int               line1()        const { return m_line1; }
  int               line2()        const { return m_line2; }
  Id                id()           const { return m_id; }
  Attr              attrs()        const { return m_attrs; }
  const StringData* name()         const { return m_name; }
  const StringData* parentName()   const { return m_parentName; }
  const StringData* docComment()   const { return m_docComment; }

  static const StringData* mangleFieldName(const StringData* recordName,
                                           const StringData* fieldName,
                                           Attr attrs);

  // PreRecordDesc contains only fields declared in the record,
  // not ones declared in parent(s)
  using FieldRange = folly::Range<const Field*>;
  FieldRange allFields() const {
    return FieldRange(m_fields.accessList(), m_fields.size());
  }

  bool isUnique() const { return m_attrs & AttrUnique; }
  bool isPersistent() const { return m_attrs & AttrPersistent; }

private:
  Unit* m_unit;
  LowPtr<NamedEntity> m_namedEntity;
  int m_line1;
  int m_line2;
  Id m_id;
  Attr m_attrs;
  LowStringPtr m_name;
  LowStringPtr m_parentName;
  LowStringPtr m_docComment;
  FieldMap m_fields;
};

using PreRecordDescPtr = AtomicSharedPtr<PreRecordDesc>;
using RecordDescPtr = AtomicSharedLowPtr<RecordDesc>;

struct RecordDesc : AtomicCountable {
  // There is nothing request specific in record fields
  using Field = PreRecordDesc::Field;
  using FieldMap = PreRecordDesc::FieldMap;
  using FieldRange = PreRecordDesc::FieldRange;

  // Record availability. See Record::availWithParent()
  enum class Avail {
    False,
    True,
    Fail
  };

  RecordDesc(PreRecordDesc* preRec, RecordDesc* parent);

  Unit* unit()                   const { return m_preRec->unit(); }
  const StringData* name()       const { return m_preRec->name(); }
  Attr attrs()                   const { return m_preRec->attrs(); }
  const StringData* parentName() const { return m_preRec->parentName();  }
  size_t stableHash()            const;

  /*
   * Whether this record is uniquely named across the codebase.
   *
   * It's legal to define multiple records in different pseudomains
   * with the same name, so long as both are not required in the same request.
   */
  bool isUnique() const { return attrs() & AttrUnique; }

  /*
   * Whether we can load this record once and persist it across requests.
   *
   * Persistence is possible when a RecordDesc is uniquely named and
   * is defined in a pseudomain that has no side-effects
   * (except other persistent definitions).
   *
   * A record which satisfies isPersistent() may not actually /be/ persistent,
   * if we had to allocate its RDS handle before we loaded the record.
   *
   * @see: recordHasPersistentRDS()
   * @implies: isUnique()
   */
  bool isPersistent() const { return attrs() & AttrPersistent; }
  bool verifyPersistent() const;

  const RecordDesc* parent() const { return m_parent.get(); }
  bool recordDescOf(const RecordDesc* rec) const;
  bool subtypeOf(const RecordDesc* rec) const { return recordDescOf(rec); }

  // Returns null if there is no common ancestor
  const RecordDesc* commonAncestor(const RecordDesc*) const;

  // Fields declared in current record as well as in its parent(s)
  size_t numFields() const { return m_fields.size(); }
  FieldRange allFields() const {
    return FieldRange(m_fields.accessList(), m_fields.size());
  }
  Slot lookupField(const StringData*) const;
  const Field& field(Slot) const;

  const PreRecordDesc* preRecordDesc() const { return m_preRec.get(); }

  /* Allocate a new RecordDesc.
   *
   * Eventually deallocated using atomicRelease(), but can go through some
   * phase changes before that (see destroy()).
   */
  static RecordDesc* newRecordDesc(PreRecordDesc* preRec, RecordDesc* parent);
  void destroy();
  /*
   * Called when the (atomic) refcount hits zero.
   *
   * The RecordDesc is completely dead at this point, and its memory is freed
   * immediately.
   */
  void atomicRelease();

  AtomicLowPtr<RecordDesc> m_next{nullptr}; // used by NamedEntity

  void setCached();
  void setRecordDescHandle(rds::Link<LowPtr<RecordDesc>,
                                     rds::Mode::NonLocal> link) const;
  rds::Handle recordHandle() const { return m_cachedRecordDesc.handle(); }

  /*
   * Check whether a RecordDesc from a previous request is available
   * to be defined.
   * The caller should check that it has the same PreRecordDesc that is being
   * defined. Being available means that the parent is available
   * (or become defined via autoload, if tryAutoload is true).
   *
   * @returns: Avail::True:  if it's available
   *           Avail::Fail:  if the parent is not defined at all at this point
   *           Avail::False: if the parent is defined but does not correspond
   *                         to this particular RecordDesc*
   *
   * The parent parameter is used for two purposes: first, it lets us avoid
   * looking up the active parent record for each potential RecordDesc*;
   * and second, it is used on Fail to return the problem record so the caller
   * can report the error correctly.
   */
  Avail availWithParent(RecordDesc*& parent, bool tryAutoload = false) const;
  bool isZombie() const { return !m_cachedRecordDesc.bound(); }

  /*
   * Return true, and set the m_serialized flag, iff this RecordDesc hasn't
   * been serialized yet (see prof-data-serialize.cpp).
   *
   * Not thread safe - caller is responsible for any necessary locking.
   */
  bool serialize() const;

  /*
   * Return true if this RecordDesc was already serialized.
   */
  bool wasSerialized() const;

private:
  void setParent();
  void setFields();

  RecordDescPtr m_parent;
  PreRecordDescPtr m_preRec;
  FieldMap m_fields;

  mutable rds::Link<LowPtr<RecordDesc>, rds::Mode::NonLocal> m_cachedRecordDesc;

  mutable bool m_serialized : 1;
};

inline bool recordHasPersistentRDS(const RecordDesc* rec) {
  return rec && rds::isPersistentHandle(rec->recordHandle());
}

extern Mutex g_recordsMutex;

///////////////////////////////////////////////////////////////////////////////
}

