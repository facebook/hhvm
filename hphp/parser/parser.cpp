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
#include "parser.h"

#include <folly/Conv.h>
#include <folly/Format.h>

#include "hphp/util/hash.h"

#include <boost/algorithm/string/predicate.hpp>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool ParserBase::IsClosureName(const std::string &name) {
  return boost::istarts_with(name, "closure$");
}

std::string ParserBase::newClosureName(
    const std::string &namespaceName,
    const std::string &className,
    const std::string &funcName) {
  // Closure names must be globally unique.  The easiest way to do
  // this is include a hash of the filename.
  int64_t hash = hash_string_cs(m_fileName, strlen(m_fileName));

  std::string name = "Closure$";
  if (!className.empty()) {
    name += className + "::";
  } else if (!namespaceName.empty()) {
    // If className is present, it already includes the namespace
    name += namespaceName + "\\";
  }
  name += funcName;

  int id = ++m_seenClosures[name];
  if (id > 1) {
    // we've seen the same name before, uniquify
    name = name + '#' + std::to_string(id);
  }

  folly::format(&name, ";{}", hash);

  return name;
}

///////////////////////////////////////////////////////////////////////////////

ParserBase::ParserBase(Scanner &scanner, const char *fileName)
    : m_scanner(scanner), m_fileName(fileName) {
  if (m_fileName == nullptr) m_fileName = "";

  // global scope
  m_labelInfos.reserve(3);
  m_labelInfos.resize(1);
  pushLabelScope();
}

ParserBase::~ParserBase() {
}

std::string ParserBase::getMessage(bool filename /* = false */,
                                   bool rawPosWhenNoError /* = false */
                                  ) const {
  std::string ret = m_scanner.getError();

  if (!ret.empty()) {
    ret += " ";
  }
  if (!ret.empty() || rawPosWhenNoError) {
    ret += getMessage(m_scanner.getLocation()->r, filename);
  }

  return ret;
}

std::string ParserBase::getMessage(const Location::Range& loc,
                                   bool filename /* = false */) const {
  int line = loc.line1;
  int column = loc.char1;
  std::string ret = "(";
  if (filename) {
    ret += std::string("File: ") + file() + ", ";
  }
  ret += std::string("Line: ") + folly::to<std::string>(line);
  ret += ", Char: " + folly::to<std::string>(column) + ")";
  return ret;
}

const Location::Range& ParserBase::getRange() const {
  return m_loc.r;
}

///////////////////////////////////////////////////////////////////////////////
// T_FUNCTION related functions

void ParserBase::pushFuncLocation() {
  m_funcLocs.push_back(getRange());
}

Location::Range ParserBase::popFuncLocation() {
  assert(!m_funcLocs.empty());
  auto loc = m_funcLocs.back();
  m_funcLocs.pop_back();
  return loc;
}

void ParserBase::pushClass(bool isXhpClass) {
  m_classes.push_back(isXhpClass);
}

bool ParserBase::peekClass() {
  assert(!m_classes.empty());
  return m_classes.back();
}

void ParserBase::popClass() {
  m_classes.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
// typevar scopes

void ParserBase::pushTypeScope() {
  m_typeScopes.push_back(m_typeVars);
  m_typeVars.clear();
}

void ParserBase::popTypeScope() {
  m_typeScopes.pop_back();
}

void ParserBase::addTypeVar(const std::string &name) {
  m_typeVars.insert(name);
}

bool ParserBase::isTypeVar(const std::string &name) {
  for (TypevarScopeStack::iterator iter = m_typeScopes.begin();
       iter != m_typeScopes.end();
       iter++) {
    if (iter->find(name) != iter->end()) {
      return true;
    }
  }
  return false;
}

bool ParserBase::isTypeVarInImmediateScope(const std::string &name) {
  int currentScope = m_typeScopes.size() - 1;
  return m_typeScopes[currentScope].find(name)
         != m_typeScopes[currentScope].end();
}

///////////////////////////////////////////////////////////////////////////////
// checks GOTO label syntax

void ParserBase::pushLabelInfo() {
  m_labelInfos.resize(m_labelInfos.size() + 1);
  pushLabelScope();
}

void ParserBase::pushLabelScope() {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  info.scopes.push_back(++info.scopeId);
}

void ParserBase::popLabelScope() {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  info.scopes.pop_back();
}

void ParserBase::addLabel(const std::string &label,
                          const Location::Range& loc,
                          ScannerToken *stmt) {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  if (info.labels.find(label) != info.labels.end()) {
    error("Label '%s' already defined: %s", label.c_str(),
          getMessage().c_str());
    invalidateLabel(extractStatement(stmt));
    return;
  }
  assert(!info.scopes.empty());
  LabelStmtInfo labelInfo;
  labelInfo.scopeId         = info.scopes.back();
  labelInfo.stmt            = extractStatement(stmt);
  labelInfo.loc             = loc;
  labelInfo.inTryCatchBlock = false;
  info.labels[label]        = labelInfo;
}

void ParserBase::addGoto(const std::string &label,
                         const Location::Range& loc,
                         ScannerToken *stmt) {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  GotoInfo gotoInfo;
  gotoInfo.label  = label;
  gotoInfo.scopes = info.scopes;
  gotoInfo.loc    = loc;
  gotoInfo.stmt   = extractStatement(stmt);
  info.gotos.push_back(gotoInfo);
}

void ParserBase::popLabelInfo() {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  LabelMap labels = info.labels; // shallow copy

  for (unsigned int i = 0; i < info.gotos.size(); i++) {
    const GotoInfo &gotoInfo = info.gotos[i];
    LabelMap::const_iterator iter = info.labels.find(gotoInfo.label);
    if (iter == info.labels.end()) {
      invalidateGoto(gotoInfo.stmt, UndefLabel);
      error("'goto' to undefined label '%s': %s",
            gotoInfo.label.c_str(), getMessage(gotoInfo.loc).c_str());
      continue;
    }
    const LabelStmtInfo &labelInfo = iter->second;
    int labelScopeId = labelInfo.scopeId;
    bool found = false;
    for (int j = gotoInfo.scopes.size() - 1; j >= 0; j--) {
      if (labelScopeId == gotoInfo.scopes[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      invalidateGoto(gotoInfo.stmt, InvalidBlock);
      error("'goto' into loop or switch statement "
            "is disallowed: %s", getMessage(gotoInfo.loc).c_str());
      continue;
    } else {
      labels.erase(gotoInfo.label);
    }
  }

  // now invalidate all un-used labels
  for (LabelMap::const_iterator it(labels.begin());
       it != labels.end();
       ++it) {
    invalidateLabel(it->second.stmt);
  }

  m_labelInfos.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
}
