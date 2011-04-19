/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <util/hash.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Mutex ParserBase::s_mutex;
std::map<int64, int> ParserBase::s_closureIds;

void ParserBase::Reset() {
  Lock lock(s_mutex);
  s_closureIds.clear();
}

///////////////////////////////////////////////////////////////////////////////

ParserBase::ParserBase(Scanner &scanner, const char *fileName)
    : m_scanner(scanner), m_fileName(fileName), m_nsState(SeenNothing) {
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
// T_FUNCTION related functions

void ParserBase::pushFuncLocation() {
  m_funcLocs.push_back(getLocation());
}

LocationPtr ParserBase::popFuncLocation() {
  ASSERT(!m_funcLocs.empty());
  LocationPtr loc = m_funcLocs.back();
  m_funcLocs.pop_back();
  return loc;
}

std::string ParserBase::getClosureName() {
  int64 h = hash_string_cs(m_fileName, strlen(m_fileName));
  int closureId;
  {
    Lock lock(s_mutex);
    int &id = s_closureIds[h];
    closureId = ++id;
  }

  string ret;
  ret = "0";
  ret += lexical_cast<string>(h);
  ret += "_";
  ret += lexical_cast<string>(closureId);
  return ret;
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
    error("Label '%s' already defined: %s", label.c_str(),
          getMessage().c_str());
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
    // "yield" generates unlimited goto, so no need to check scoping
    if (gotoInfo.label.find(YIELD_LABEL_PREFIX) == 0) {
      continue;
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
// namespace support

void ParserBase::nns(bool declare /* = false */) {
  if (m_nsState == SeenNamespaceStatement) {
    error("No code may exist outside of namespace {}: %s",
          getMessage().c_str());
    return;
  }
  if (m_nsState == SeenNothing && !declare) {
    m_nsState = SeenNonNamespaceStatement;
  }
}

void ParserBase::onNamespaceStart(const std::string &ns) {
  if (m_nsState == SeenNonNamespaceStatement) {
    error("Namespace declaration statement has to be the very first "
          "statement in the script: %s", getMessage().c_str());
    return;
  }
  m_nsState = InsideNamespace;

  m_namespace = ns;
}

void ParserBase::onNamespaceEnd() {
  m_nsState = SeenNamespaceStatement;
}

void ParserBase::onUse(const std::string &ns, const std::string &as) {
  if (m_aliases.find(as) != m_aliases.end()) {
    error("Cannot use %s as %s because the name is already in use: %s",
          ns.c_str(), as.c_str(), getMessage().c_str());
    return;
  }
  string key = as;
  if (key.empty()) {
    size_t pos = ns.rfind(NAMESPACE_SEP);
    if (pos == string::npos) {
      key = ns;
    } else {
      key = ns.substr(pos + 1);
    }
  }
  m_aliases[key] = ns;
}

std::string ParserBase::nsDecl(const std::string &name) {
  if (m_namespace.empty()) {
    return name;
  }
  return m_namespace + NAMESPACE_SEP + name;
}

std::string ParserBase::resolve(const std::string &ns, bool cls) {
  // try import rules first
  string alias = ns;
  size_t pos = ns.find(NAMESPACE_SEP);
  if (pos != string::npos) {
    alias = ns.substr(0, pos);
  }
  hphp_string_imap<std::string>::const_iterator iter = m_aliases.find(alias);
  if (iter != m_aliases.end()) {
    if (pos != string::npos) {
      return iter->second + ns.substr(pos);
    }
    return iter->second;
  }

  // if qualified name, prepend current namespace
  if (pos != string::npos) {
    return nsDecl(ns);
  }

  // unqualified name in global namespace
  if (m_namespace.empty()) {
    return ns;
  }

  // unqualified class name always prefixed with NAMESPACE_SEP
  if (cls) {
    return m_namespace + NAMESPACE_SEP + ns;
  }

  // unqualified function name needs leading NAMESPACE_SEP to indicate this
  // needs runtime resolution
  return string("") + NAMESPACE_SEP + m_namespace + NAMESPACE_SEP + ns;
}

///////////////////////////////////////////////////////////////////////////////
}
