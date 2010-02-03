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

#include "json.h"

using namespace std;

namespace HPHP { namespace JSON {
///////////////////////////////////////////////////////////////////////////////
// statics

string Escape(const char *s) {
  string ret;
  for (const char *p = s; *p; p++) {
    switch (*p) {
    case '\r': ret += "\\r";  break;
    case '\n': ret += "\\n";  break;
    case '\t': ret += "\\t";  break;
    case '/':  ret += "\\/";  break;
    case '\"': ret += "\\\""; break;
    case '\\': ret += "\\\\"; break;
    default:   ret += *p;     break;
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Name

Name::Name(const char *name) {
  ASSERT(name && *name);
  m_name = name;
}

Name::Name(const std::string &name) {
  ASSERT(!name.empty());
  m_name = name;
}

void Name::serialize(OutputStream &out) const {
  out.raw() << "\"" << m_name << "\":";
}

///////////////////////////////////////////////////////////////////////////////
// OutputStream

OutputStream &OutputStream::operator<< (int v) {
  m_out << v;
  return *this;
}

OutputStream &OutputStream::operator<< (const char *v) {
  m_out << "\"" << Escape(v) << "\"";
  return *this;
}

OutputStream &OutputStream::operator<< (const std::string &v) {
  m_out << "\"" << Escape(v.c_str()) << "\"";
  return *this;
}

OutputStream &OutputStream::operator<< (const Name &v) {
  v.serialize(*this);
  return *this;
}

OutputStream &OutputStream::operator<< (const ISerializable &v) {
  v.serialize(*this);
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
}}
