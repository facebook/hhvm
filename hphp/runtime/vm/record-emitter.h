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

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/array-data.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/record.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
struct UnitEmitter;

struct RecordEmitter {

  struct Field {
    Field() = default;

    Field(const RecordEmitter* re,
          const StringData* n,
          Attr attrs,
          const StringData* userType,
          const TypeConstraint& typeConstraint,
          const StringData* docComment,
          const TypedValue* val,
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
    UserAttributeMap userAttributes() const { return m_userAttributes; }

    template<class SerDe> void serde(SerDe& sd) {
      sd(m_name)
        (m_mangledName)
        (m_attrs)
        (m_userType)
        (m_docComment)
        (m_val)
        (m_repoAuthType)
        (m_typeConstraint)
        (m_userAttributes)
        ;
    }

  private:
    friend RecordEmitter;
    void resolveArray(const RecordEmitter* re) {
      m_repoAuthType.resolveArray(re->ue());
    }

  private:
    LowStringPtr m_name{nullptr};
    LowStringPtr m_mangledName{nullptr};
    Attr m_attrs{AttrNone};
    LowStringPtr m_userType{nullptr};
    LowStringPtr m_docComment{nullptr};
    TypedValue m_val{};
    RepoAuthType m_repoAuthType{};
    TypeConstraint m_typeConstraint{};
    UserAttributeMap m_userAttributes{};
  };

  using FieldMap = IndexedStringMap<Field, Slot>;

  RecordEmitter(UnitEmitter& ue, Id id, const std::string& name);

  void init(int line1, int line2, Attr attrs,
            const StringData* parentName,
            const StringData* docComment);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  const StringData* parentName() const { return m_parent; }
  Attr attrs() const { return m_attrs; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }
  UserAttributeMap userAttributes() const { return m_userAttributes; }
  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }

  Id id() const { return m_id; }

  PreRecordDesc* create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line1, m_line2);
  }
  const StringData* docComment() const { return m_docComment; }

  const FieldMap::Builder& fieldMap() const { return m_fieldMap; }
  using UpperBoundVec = CompactVector<TypeConstraint>;
  void addUpperBound(const StringData*, const UpperBoundVec&) {}
  bool hasUpperBound(const StringData*) { return false; }
  bool addField(const StringData* n,
                Attr attrs,
                const StringData* userType,
                const TypeConstraint& typeConstraint,
                const UpperBoundVec&, // unused
                const StringData* docComment,
                const TypedValue* val,
                RepoAuthType,
                UserAttributeMap);

  private:
    UnitEmitter& m_ue;
    int m_line1;
    int m_line2;
    LowStringPtr m_name;
    Attr m_attrs;
    LowStringPtr m_parent;
    LowStringPtr m_docComment;
    Id m_id;
    UserAttributeMap m_userAttributes;
    FieldMap::Builder m_fieldMap;
};

///////////////////////////////////////////////////////////////////////////////
}
