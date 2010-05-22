/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_VARIABLE_UNSERIALIZER_H__
#define __HPHP_VARIABLE_UNSERIALIZER_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class VariableUnserializer {
public:
  VariableUnserializer(std::istream &in) : m_in(in), m_key(false) {}

  Variant unserialize() {
    Variant v;
    v.unserialize(this);
    return v;
  }

  Variant unserializeKey() {
    m_key = true;
    Variant v;
    v.unserialize(this);
    m_key = false;
    return v;
  }

  std::istream &in() const {
    return m_in;
  }
  void add(Variant* v) {
    if (!m_key) {
      m_refs.push_back(v);
    }
  }
  Variant *get(int id) {
    if (id <= 0  || id > (int)m_refs.size()) return NULL;
    return m_refs[id-1];
  }

 private:
  std::istream &m_in;
  std::vector<Variant*> m_refs;
  bool m_key;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIABLE_UNSERIALIZER_H__
