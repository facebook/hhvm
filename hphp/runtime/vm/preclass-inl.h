/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

inline bool PreClass::hasMethod(const StringData* methName) const {
  return m_methods.contains(methName);
}

inline bool PreClass::hasProp(const StringData* propName) const {
  return m_properties.contains(propName);
}

inline Func* PreClass::lookupMethod(const StringData* methName) const {
  Func* f = m_methods.lookupDefault(methName, nullptr);
  assert(f != nullptr);
  return f;
}

inline const PreClass::Prop*
PreClass::lookupProp(const StringData* propName) const {
  Slot s = m_properties.findIndex(propName);
  assert(s != kInvalidSlot);
  return &m_properties[s];
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
  uintptr_t packCR(const StringData* req, bool isExtends) {
    auto reqPtr = reinterpret_cast<uintptr_t>(req);
    return isExtends ? (reqPtr | 0x1) : reqPtr;
  }
}

inline
PreClass::ClassRequirement::ClassRequirement()
  : m_word(0)
{}

inline
PreClass::ClassRequirement::ClassRequirement(const StringData* req,
                                             bool isExtends)
  : m_word(packCR(req, isExtends))
{}

/*
 * Accessors.
 */
inline const StringData* PreClass::ClassRequirement::name() const {
  return reinterpret_cast<const StringData*>(m_word & ~0x1);
}

inline bool PreClass::ClassRequirement::is_extends() const {
  return m_word & 0x1;
}

inline bool PreClass::ClassRequirement::is_implements() const {
  return !is_extends();
}

/*
 * Deserialization.
 */
template<class SerDe>
typename std::enable_if<SerDe::deserializing>::type
PreClass::ClassRequirement::serde(SerDe& sd) {
  const StringData* sd_name;
  bool sd_is_extends;
  sd(sd_name)(sd_is_extends);
  m_word = packCR(sd_name, sd_is_extends);
}

/*
 * Serialization.
 */
template<class SerDe>
typename std::enable_if<!SerDe::deserializing>::type
PreClass::ClassRequirement::serde(SerDe& sd) {
  sd(name())(is_extends());
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::Const.

/*
 * Ser(ialization)-De(serialization).
 */
template<class SerDe>
inline void PreClass::Const::serde(SerDe& sd) {
  TypedValue sd_tv = m_val;
  auto sd_modifiers = m_val.constModifiers();
  sd(m_name)
    (m_phpCode)
    (sd_tv)
    (sd_modifiers);

  if (SerDe::deserializing) {
    // tvCopy inlined here to avoid header dependency issues
    m_val.m_data.num = sd_tv.m_data.num;
    m_val.m_type = sd_tv.m_type;
    m_val.constModifiers() = sd_modifiers;
  }
}

///////////////////////////////////////////////////////////////////////////////

}
