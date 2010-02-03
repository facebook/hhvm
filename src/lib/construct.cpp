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

#include <lib/construct.h>
#include <lib/parser/parser.h>
#include <util/util.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/function_scope.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/dependency_graph.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////

Construct::Construct(LocationPtr loc)
 : m_extra(NULL), m_loc(loc), m_fileLevel(false), m_topLevel(false) {
}

std::string Construct::getText(bool useCache /* = false */) {
  std::string &text = m_text;
  if (useCache && !text.empty()) return text;
  ostringstream o;
  CodeGenerator cg(&o, CodeGenerator::PickledPHP);
  outputPHP(cg, AnalysisResultPtr()); // we knew PickledPHP won't use ar
  text = o.str();
  return text;
}

void Construct::serialize(JSON::OutputStream &out) const {
  out.raw() << "[\"" << m_loc->file << "\"," <<
    m_loc->line0 << "," << m_loc->char0 << "," <<
    m_loc->line1 << "," << m_loc->char1 << "]";
}

void Construct::addUserFunction(AnalysisResultPtr ar,
                                const std::string &name,
                                bool strong /* = true */) {
  if (!name.empty()) {
    FunctionScopePtr func = ar->findFunction(name);
    if (func && func->isUserFunction()) {
      ar->getDependencyGraph()->add
        (DependencyGraph::KindOfProgramUserFunction,
         ar->getName(), func->getName(), func->getStmt());
      ar->addCallee(func->getStmt());
    }
    if (strong && ar->getPhase() == AnalysisResult::AnalyzeAll) {
      FunctionScopePtr func = ar->getFunctionScope();
      ar->getFileScope()->addFunctionDependency(ar, name, func &&
                                                func->isInlined());
    }
  }
}

void Construct::addUserClass(AnalysisResultPtr ar,
                             const std::string &name,
                             bool strong /* = true */) {
  if (!name.empty()) {
    ClassScopePtr cls = ar->findClass(name);
    if (cls && cls->isUserClass()) {
      ar->getDependencyGraph()->add(DependencyGraph::KindOfProgramUserClass,
                                    ar->getName(),
                                    cls->getName(), cls->getStmt());
      ar->addCallee(cls->getStmt());
    }
    if (strong && !ar->isFirstPass()) {
      ar->getFileScope()->addClassDependency(ar, name);
    }
  }
}

bool Construct::isErrorSuppressed(CodeError::ErrorType e) const {
  std::vector<CodeError::ErrorType> &suppressedErrors =
    getExtra()->suppressedErrors;
  for (unsigned int i = 0; i < suppressedErrors.size(); i++) {
    if (suppressedErrors[i] == e) {
      return true;
    }
  }
  return false;
}

void Construct::addHphpNote(const std::string &s) {
  ExtraData *extra = getExtra();
  if (s.find("C++") == 0) {
    extra->embedded += s.substr(3);
    extra->hphpNotes.insert("C++");
  } else {
    extra->hphpNotes.insert(s);
  }
}

void Construct::printSource(CodeGenerator &cg) {
  if (m_loc) {
    cg.printf("/* SRC: %s line %d */\n", m_loc->file, m_loc->line1);
  }
}
