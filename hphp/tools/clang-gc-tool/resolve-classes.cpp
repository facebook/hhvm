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

#undef __GXX_RTTI

#include "hphp/tools/clang-gc-tool/resolve-classes.h"
#include <iostream>

namespace HPHP {

using clang::ASTContext;
using clang::CXXRecordDecl;
using clang::FieldDecl;
using clang::ClassTemplateDecl;
using clang::CallExpr;
using clang::isa;
using clang::cast;

ResolveClassesVisitor::ResolveClassesVisitor(
  ASTContext& context,
  const std::set<std::string>& needsScanMethod,
  DeclSet& gcClasses,
  const std::set<std::string>& hasScanMethodNames,
  DeclSet& hasScanMethod,
  const std::set<std::string>& badContainerNames,
  DeclSet& badContainers,
  bool verbose
) : PluginUtil(context),
    m_needsScanMethod(needsScanMethod),
    m_gcClasses(gcClasses),
    m_hasScanMethodNames(hasScanMethodNames),
    m_hasScanMethod(hasScanMethod),
    m_badContainerNames(badContainerNames),
    m_badContainers(badContainers),
    m_verbose(verbose)
{ }

ResolveClassesVisitor::~ResolveClassesVisitor() {
  if (m_verbose) {
    for (const auto& name : m_needsScanMethod) {
      warning("Can't resolve decl for gc class '%s'", name);
    }
    for (const auto& name : m_hasScanMethodNames) {
      warning("Can't resolve decl for has scan method class '%s'", name);
    }
    for (const auto& name : m_badContainerNames) {
      warning("Can't resolve decl for has bad container class '%s'", name);
    }
  }
}

// See if decl belongs to any of the predefined sets.  If so, store it
// in the appropriate DeclSet.
void ResolveClassesVisitor::resolveDecl(const CXXRecordDecl* decl) {
  assert(isa<CXXRecordDecl>(decl));

  decl = decl->getCanonicalDecl();

  if (!decl->hasDefinition()) return;

  auto name = getName(decl);
  if (isTemplate(decl)) {
    decl = getTemplateDef(decl)->getTemplatedDecl()->getCanonicalDecl();
    name = name.substr(0, name.find('<'));
  }

  if (!exists(m_gcClasses, decl) && exists(m_needsScanMethod, name)) {
    m_gcClasses.insert(decl);
    m_needsScanMethod.erase(m_needsScanMethod.find(name));
  }
  if (!exists(m_badContainers, decl) && exists(m_badContainerNames, name)) {
    m_badContainers.insert(decl);
    m_badContainerNames.erase(m_badContainerNames.find(name));
  }
  if (!exists(m_hasScanMethod, decl) && exists(m_hasScanMethodNames, name)) {
    m_hasScanMethod.insert(decl);
    m_hasScanMethodNames.erase(m_hasScanMethodNames.find(name));
  }
}

bool ResolveClassesVisitor::VisitFieldDecl(FieldDecl* decl) {
  auto record = decl->getType()->getAsCXXRecordDecl();
  if (record && !record->isAnonymousStructOrUnion()) {
    resolveDecl(record);
  }
  return true;
}

bool ResolveClassesVisitor::VisitClassTemplateDecl(ClassTemplateDecl* tdecl) {
  if (tdecl->isThisDeclarationADefinition()) {
    resolveDecl(tdecl->getTemplatedDecl()->getCanonicalDecl());
  }
  return true;
}

bool ResolveClassesVisitor::VisitCXXRecordDecl(CXXRecordDecl* decl) {
  if (!decl->isAnonymousStructOrUnion()) {
    resolveDecl(decl);
  }
  return true;
}

}
