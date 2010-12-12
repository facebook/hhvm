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

#include "parser.h"

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ParserBase::ParserBase(Scanner &scanner, const char *fileName)
    : m_scanner(scanner), m_fileName(fileName) {
  if (m_fileName == NULL) m_fileName = "";
}

ParserBase::~ParserBase() {
}

std::string ParserBase::getMessage(bool filename /* = false */) const {
  int line = m_scanner.getLocation()->line1;
  int column = m_scanner.getLocation()->char1;

  string ret = m_scanner.getError();
  ret += " (";
  if (filename) {
    ret += string("File: ") + file() + ", ";
  }
  ret += string("Line: ") + lexical_cast<string>(line);
  ret += ", Char: " + lexical_cast<string>(column) + ")";
  return ret;
}

LocationPtr ParserBase::getLocation() const {
  LocationPtr location(new Location());
  location->file  = file();
  location->line0 = line0();
  location->char0 = char0();
  location->line1 = line1();
  location->char1 = char1();
  return location;
}

void ParserBase::pushFuncLocation() {
  m_funcLocs.push_back(getLocation());
}

LocationPtr ParserBase::popFuncLocation() {
  ASSERT(!m_funcLocs.empty());
  LocationPtr loc = m_funcLocs.back();
  m_funcLocs.pop_back();
  return loc;
}

///////////////////////////////////////////////////////////////////////////////
}
