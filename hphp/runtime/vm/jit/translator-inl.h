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

#ifndef incl_HPHP_TRANSLATOR_INL_H_
#error "translator-inl.h should only be included by translator.h"
#endif

namespace HPHP { namespace JIT {
///////////////////////////////////////////////////////////////////////////////

inline JIT::IRTranslator* Translator::irTrans() const {
  return m_irTrans.get();
}

inline ProfData* Translator::profData() const {
  return m_profData.get();
}

inline const SrcDB& Translator::getSrcDB() const {
  return m_srcDB;
}

inline SrcRec* Translator::getSrcRec(const SrcKey& sk) {
  // XXX: Add a insert-or-find primitive to THM.
  if (SrcRec* r = m_srcDB.find(sk)) return r;
  assert(s_writeLease.amOwner());
  return m_srcDB.insert(sk);
}

///////////////////////////////////////////////////////////////////////////////

inline TransKind Translator::mode() const {
  return m_mode;
}

inline void Translator::setMode(TransKind mode) {
  m_mode = mode;
}

inline bool Translator::useAHot() const {
  return m_useAHot;
}

inline void Translator::setUseAHot(bool val) {
  m_useAHot = val;
}

///////////////////////////////////////////////////////////////////////////////

inline bool Translator::isTransDBEnabled() {
  return debug || RuntimeOption::EvalDumpTC;
}

inline const TransRec* Translator::getTransRec(TCA tca) const {
  if (!isTransDBEnabled()) return nullptr;

  TransDB::const_iterator it = m_transDB.find(tca);
  if (it == m_transDB.end()) {
    return nullptr;
  }
  if (it->second >= m_translations.size()) {
    return nullptr;
  }
  return &m_translations[it->second];
}

inline const TransRec* Translator::getTransRec(TransID transId) const {
  if (!isTransDBEnabled()) return nullptr;

  always_assert(transId < m_translations.size());
  return &m_translations[transId];
}

inline TransID Translator::getCurrentTransID() const {
  return m_translations.size();
}

///////////////////////////////////////////////////////////////////////////////

inline Lease& Translator::WriteLease() {
  return s_writeLease;
}

///////////////////////////////////////////////////////////////////////////////
}}
