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

  // global scope
  m_labelInfos.reserve(3);
  m_labelInfos.resize(1);
  pushLabelScope();
}

ParserBase::~ParserBase() {
}

std::string ParserBase::getMessage(bool filename /* = false */) const {
  string ret = m_scanner.getError();
  if (!ret.empty()) {
    ret += " ";
  }
  ret += getMessage(m_scanner.getLocation(), filename);
  return ret;
}

std::string ParserBase::getMessage(Location *loc,
                                   bool filename /* = false */) const {
  int line = loc->line1;
  int column = loc->char1;
  string ret = "(";
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

///////////////////////////////////////////////////////////////////////////////
// correctly set T_FUNCTION source locations

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
// checks GOTO label syntax

void ParserBase::pushLabelInfo() {
  m_labelInfos.resize(m_labelInfos.size() + 1);
  pushLabelScope();
}

void ParserBase::pushLabelScope() {
  ASSERT(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  info.scopes.push_back(++info.scopeId);
}

void ParserBase::popLabelScope() {
  ASSERT(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  info.scopes.pop_back();
}

void ParserBase::addLabel(const std::string &label) {
  ASSERT(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  if (info.labels.find(label) != info.labels.end()) {
    error("Label '%s' already defined", label.c_str());
    return;
  }
  ASSERT(!info.scopes.empty());
  info.labels[label] = info.scopes.back();
}

void ParserBase::addGoto(const std::string &label, LocationPtr loc) {
  ASSERT(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  GotoInfo gotoInfo;
  gotoInfo.label = label;
  gotoInfo.scopes = info.scopes;
  gotoInfo.loc = loc;
  info.gotos.push_back(gotoInfo);
}

void ParserBase::popLabelInfo() {
  ASSERT(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  for (unsigned int i = 0; i < info.gotos.size(); i++) {
    const GotoInfo &gotoInfo = info.gotos[i];
    LabelMap::const_iterator iter = info.labels.find(gotoInfo.label);
    if (iter == info.labels.end()) {
      error("'goto' to undefined label '%s': %s",
            gotoInfo.label.c_str(), getMessage(gotoInfo.loc.get()).c_str());
      return;
    }
    int labelScopeId = iter->second;
    bool found = false;
    for (int j = gotoInfo.scopes.size() - 1; j >= 0; j--) {
      if (labelScopeId == gotoInfo.scopes[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      error("'goto' into loop or switch statement or try/catch block "
            "is disallowed: %s", getMessage(gotoInfo.loc.get()).c_str());
      return;
    }
  }
  m_labelInfos.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
}
