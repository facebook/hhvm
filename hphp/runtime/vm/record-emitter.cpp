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
#include "hphp/runtime/vm/record-emitter.h"

#include <limits>
#include <sstream>

#include <folly/Memory.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/record.h"

namespace HPHP {

RecordEmitter::Field::Field(const RecordEmitter* re,
                            const StringData* n,
                            Attr attrs,
                            const StringData* userType,
                            const TypeConstraint& typeConstraint,
                            const StringData* docComment,
                            const TypedValue* val,
                            RepoAuthType repoAuthType,
                            UserAttributeMap userAttributes)
  : m_name(n)
  , m_attrs(attrs)
  , m_userType(userType)
  , m_docComment(docComment)
  , m_repoAuthType(repoAuthType)
  , m_typeConstraint(typeConstraint)
  , m_userAttributes(userAttributes)
{
  m_mangledName = PreRecordDesc::mangleFieldName(re->name(), n, attrs);
  memcpy(&m_val, val, sizeof(TypedValue));
}

RecordEmitter::RecordEmitter(UnitEmitter& ue, Id id, const std::string& name)
  : m_ue(ue)
  , m_name(makeStaticString(name))
  , m_id(id) {}

void RecordEmitter::init(int line1, int line2, Attr attrs,
                         const StringData* parentName,
                         const StringData* docComment) {
  m_line1 = line1;
  m_line2 = line2;
  m_attrs = attrs;
  m_parent = parentName;
  m_docComment = docComment;
}

PreRecordDesc* RecordEmitter::create(Unit& unit) const {
  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      !RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
    attrs = Attr(attrs & ~AttrPersistent);
  }
  assertx(attrs & AttrPersistent || SystemLib::s_inited);

  auto  rec = std::make_unique<PreRecordDesc>(
      &unit, m_line1, m_line2, m_name, attrs, m_parent, m_docComment, m_id);

  PreRecordDesc::FieldMap::Builder fieldBuild;
  for (unsigned i = 0; i < m_fieldMap.size(); ++i) {
    const Field& field = m_fieldMap[i];
    fieldBuild.add(field.name(), PreRecordDesc::Field(rec.get(),
                                                      field.name(),
                                                      field.attrs(),
                                                      field.userType(),
                                                      field.typeConstraint(),
                                                      field.docComment(),
                                                      field.val(),
                                                      field.repoAuthType(),
                                                      field.userAttributes()));
  }
  rec->m_fields.create(fieldBuild);
  return rec.release();
}

bool RecordEmitter::addField(const StringData* n,
                             Attr attrs,
                             const StringData* userType,
                             const TypeConstraint& typeConstraint,
                             const UpperBoundVec&, // unused
                             const StringData* docComment,
                             const TypedValue* val,
                             RepoAuthType repoAuthType,
                             UserAttributeMap userAttributes) {
  assertx(typeConstraint.validForRecField());
  FieldMap::Builder::const_iterator it = m_fieldMap.find(n);
  if (it != m_fieldMap.end()) {
    return false;
  }
  RecordEmitter::Field field{
    this,
    n,
    attrs,
    userType,
    typeConstraint,
    docComment,
    val,
    repoAuthType,
    userAttributes
  };
  m_fieldMap.add(field.name(), field);
  return true;
}

template<class SerDe> void RecordEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name and a few other fields currently
  // serialized outside of this.
  sd(m_line1)
    (m_line2)
    (m_attrs)
    (m_parent)
    (m_docComment)

    (m_userAttributes)
    (m_fieldMap, [](Field f) { return f.name(); })
    ;

    if (SerDe::deserializing) {
      for (unsigned i = 0; i < m_fieldMap.size(); ++i) {
        m_fieldMap[i].resolveArray(this);
      }
    }
}

template void RecordEmitter::serdeMetaData<>(BlobDecoder&);
template void RecordEmitter::serdeMetaData<>(BlobEncoder&);

} // HPHP
