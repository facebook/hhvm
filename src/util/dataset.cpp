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

#include "dataset.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *DataSet::getStringField(int field) const {
  const char *value = getField(field);
  return value ? value : "";
}

int DataSet::getIntField(int field) const {
  const char *value = getField(field);
  return value ? atoi(value) : 0;
}

unsigned int DataSet::getUIntField(int field) const {
  const char *value = getField(field);
  return value ? atoi(value) : 0;
}

long long DataSet::getInt64Field(int field) const {
  const char *value = getField(field);
  return value ? atoll(value) : 0;
}

unsigned long long DataSet::getUInt64Field(int field) const {
  const char *value = getField(field);
  return value ? atoll(value) : 0;
}

std::string DataSet::getRowString() const {
  string ret;

  int nonameCount = 0;
  for (int i = 0; i < getColCount(); i++) {
    if (i > 0) ret += ", ";
    ret += "'";
    const char *fieldName = getFields()[i].name;
    if (fieldName && *fieldName) {
      ret += fieldName;
    } else {
      ret += "Noname";
      ret += boost::lexical_cast<string>(nonameCount++);
    }
    ret += "' => ";
    const char *s = getField(i);
    if (s) {
      ret += "'";
      ret += escape(s);
      ret += "'";
    } else {
      ret += "NULL";
    }
  }
  return ret;
}

std::string DataSet::escape(const char *s) {
  string ret;
  for (const char *p = s; *p; p++) {
    switch (*p) {
    case '\'':      ret += "\\'" ;  break;
    case '\r':      ret += "\\r" ;  break;
    case '\n':      ret += "\\n" ;  break;
    case '\t':      ret += "\\t" ;  break;
    case '\\':      ret += "\\\\";  break;
    default:
      ret += *p;
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
