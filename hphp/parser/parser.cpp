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
#include "parser.h"

#include <folly/Conv.h>
#include <folly/Format.h>

#include "hphp/util/hash.h"
#include "hphp/util/bstring.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool ParserBase::IsClosureName(const std::string &name) {
  return name.size() >= 8 && bstrcaseeq(name.c_str(), "closure$", 8);
}

bool ParserBase::IsAnonymousClassName(folly::StringPiece name) {
  return name.find('$') != std::string::npos;
}

const char* ParserBase::labelScopeName(LabelScopeKind kind) {
  switch (kind) {
    case LabelScopeKind::Invalid:     break;
    case LabelScopeKind::LoopSwitch:  return "into loop or switch";
    case LabelScopeKind::Finally:     return "into finally";
    case LabelScopeKind::Using:       return "into or across using";

  }
  always_assert(false && "Invalid LabelScope");
}

std::string ParserBase::newClosureName(
    const std::string &namespaceName,
    const std::string &className,
    const std::string &funcName) {
  return newAnonClassName("Closure", namespaceName, className, funcName);
}

std::string ParserBase::newAnonClassName(
    const std::string &prefix,
    const std::string &namespaceName,
    const std::string &className,
    const std::string &funcName) {

  auto name = prefix + "$";
  if (!className.empty()) {
    name += className + "::";
  } else if (!namespaceName.empty() &&
             funcName.find('\\') == std::string::npos) {
    // If className is present, it already includes the namespace. Or
    // else we may be passed a function name already qualified by
    // namespace, and in either case we don't want to include the
    // namespace twice in the mangled class name.
    name += namespaceName + "\\";
  }
  name += funcName;

  auto const id = ++m_seenAnonClasses[name];
  if (id > 1) {
    // we've seen the same name before, uniquify
    folly::format(&name, "#{}", id);
  }

  return name;
}

///////////////////////////////////////////////////////////////////////////////

ParserBase::ParserBase(Scanner &scanner, const char *fileName)
    : m_scanner(scanner), m_fileName(fileName) {
  if (m_fileName == nullptr) m_fileName = "";

  // global scope
  m_labelInfos.reserve(3);
  pushLabelInfo();
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
  m_labelInfos.emplace_back();
  pushLabelScope(LabelScopeKind::Invalid);
}

void ParserBase::pushLabelScope(LabelScopeKind kind) {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  info.scopes.emplace_back(kind, ++info.scopeId);
}

void ParserBase::popLabelScope() {
  assertx(!m_labelInfos.empty());
  assertx(!m_labelInfos.back().scopes.empty());
  m_labelInfos.back().scopes.pop_back();
}

void ParserBase::addLabel(const std::string &label,
                          const Location::Range& loc,
                          ScannerToken *stmt) {
  assert(!m_labelInfos.empty());
  LabelInfo &info = m_labelInfos.back();
  if (info.labels.find(label) != info.labels.end()) {
    error("Label '%s' already defined: %s", label.c_str(),
          getMessage().c_str());
    return;
  }
  assert(!info.scopes.empty());
  auto const stmtInfo = LabelStmtInfo{
    extractStatement(stmt),
    info.scopes.back(),
    loc,
    false
  };
  info.labels.emplace(label, stmtInfo);
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
  assertx(info.scopes.size() == 1);
  assertx(info.scopes.back().kind == LabelScopeKind::Invalid);

  LabelMap labels = info.labels; // shallow copy

  for (unsigned int i = 0; i < info.gotos.size(); i++) {
    const GotoInfo &gotoInfo = info.gotos[i];
    LabelMap::const_iterator iter = info.labels.find(gotoInfo.label);
    if (iter == info.labels.end()) {
      error("'goto' to undefined label '%s': %s",
            gotoInfo.label.c_str(), getMessage(gotoInfo.loc).c_str());
      continue;
    }
    const LabelStmtInfo &labelInfo = iter->second;
    int labelScopeId = labelInfo.scopeInfo.id;
    bool found = false;
    for (int j = gotoInfo.scopes.size() - 1; j >= 0; j--) {
      if (labelScopeId == gotoInfo.scopes[j].id) {
        found = true;
        break;
      }
    }
    if (!found) {
      error("'goto' %s statement is disallowed: %s",
            labelScopeName(labelInfo.scopeInfo.kind),
            getMessage(gotoInfo.loc).c_str());
      continue;
    } else {
      labels.erase(gotoInfo.label);
    }
  }

  m_labelInfos.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
}
