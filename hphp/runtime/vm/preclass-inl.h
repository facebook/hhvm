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

#ifndef incl_HPHP_VM_PRECLASS_INL_H_
#error "preclass-inl.h should only be included by preclass.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline bool PreClass::isPersistent() const {
  return m_attrs & AttrPersistent;
}

inline bool PreClass::isBuiltin() const {
  return m_attrs & AttrBuiltin;
}

inline bool PreClass::hasConstant(const StringData* cnsName) const {
  return m_constants.contains(cnsName);
}

inline bool PreClass::hasMethod(const StringData* methName) const {
  return m_methods.contains(methName);
}

inline bool PreClass::hasProp(const StringData* propName) const {
  return m_properties.contains(propName);
}

inline const PreClass::Const*
PreClass::lookupConstant(const StringData* cnsName) const {
  Slot s = m_constants.findIndex(cnsName);
  assertx(s != kInvalidSlot);
  return &m_constants[s];
}

inline Func* PreClass::lookupMethod(const StringData* methName) const {
  Func* f = m_methods.lookupDefault(methName, nullptr);
  assertx(f != nullptr);
  return f;
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::TraitPrecRule.

inline
PreClass::TraitPrecRule::TraitPrecRule()
  : m_methodName(nullptr)
  , m_selectedTraitName(nullptr)
{}

inline
PreClass::TraitPrecRule::TraitPrecRule(const StringData* selectedTraitName,
                                       const StringData* methodName)
  : m_methodName(methodName)
  , m_selectedTraitName(selectedTraitName)
{}

inline void
PreClass::TraitPrecRule::addOtherTraitName(const StringData* traitName) {
  m_otherTraitNames.insert(traitName);
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::TraitAliasRule.

inline
PreClass::TraitAliasRule::TraitAliasRule()
  : m_traitName(nullptr)
  , m_origMethodName(nullptr)
  , m_newMethodName(nullptr)
  , m_modifiers(AttrNone)
{}

inline
PreClass::TraitAliasRule::TraitAliasRule(const StringData* traitName,
                                         const StringData* origMethodName,
                                         const StringData* newMethodName,
                                         Attr modifiers)
  : m_traitName(traitName)
  , m_origMethodName(origMethodName)
  , m_newMethodName(newMethodName)
  , m_modifiers(modifiers)
{}

///////////////////////////////////////////////////////////////////////////////
// PreClass::ClassRequirement.

namespace {
  inline uintptr_t
  packCR(const StringData* req, PreClass::RequirementKind reqKind) {
    auto reqPtr = reinterpret_cast<uintptr_t>(req);
    return reqPtr | reqKind;
  }
}

inline
PreClass::ClassRequirement::ClassRequirement()
  : m_word(0)
{}

inline
PreClass::ClassRequirement::ClassRequirement(const StringData* req,
                                             PreClass::RequirementKind reqKind)
  : m_word(packCR(req, reqKind))
{}

/*
 * Accessors.
 */
inline const StringData* PreClass::ClassRequirement::name() const {
  return reinterpret_cast<const StringData*>(m_word & ~0x3);
}

inline PreClass::RequirementKind PreClass::ClassRequirement::kind() const {
  return static_cast<PreClass::RequirementKind>(m_word & 0x3);
}

inline bool PreClass::ClassRequirement::is_same(
    const ClassRequirement* other) const {
  return m_word == other->m_word;
}

inline size_t PreClass::ClassRequirement::hash() const {
  return m_word;
}

/*
 * Deserialization.
 */
template<class SerDe>
typename std::enable_if<SerDe::deserializing>::type
PreClass::ClassRequirement::serde(SerDe& sd) {
  const StringData* sd_name;
  RequirementKind sd_reqKind;
  sd(sd_name)(sd_reqKind);
  m_word = packCR(sd_name, sd_reqKind);
}

/*
 * Serialization.
 */
template<class SerDe>
typename std::enable_if<!SerDe::deserializing>::type
PreClass::ClassRequirement::serde(SerDe& sd) {
  sd(name())(kind());
}

///////////////////////////////////////////////////////////////////////////////

}
