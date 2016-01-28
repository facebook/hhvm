/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
using clang::ClassTemplateSpecializationDecl;
using clang::CallExpr;
using clang::isa;
using clang::cast;

ResolveClassesVisitor::ResolveClassesVisitor(
  ASTContext& context,
  const std::set<std::string>& needsScanMethod,
  DeclSet& gcClasses,
  const std::set<std::string>& gcContainerNames,
  DeclSet& gcContainers,
  const std::set<std::string>& hasScanMethodNames,
  DeclSet& hasScanMethod,
  const std::set<std::string>& ignoredNames,
  DeclSet& ignoredClasses,
  const std::set<std::string>& badContainerNames,
  DeclSet& badContainers,
  bool verbose
) : PluginUtil(context),
    m_needsScanMethod(needsScanMethod),
    m_gcClasses(gcClasses),
    m_gcContainerNames(gcContainerNames),
    m_gcContainers(gcContainers),
    m_hasScanMethodNames(hasScanMethodNames),
    m_hasScanMethod(hasScanMethod),
    m_ignoredNames(ignoredNames),
    m_ignoredClasses(ignoredClasses),
    m_badContainerNames(badContainerNames),
    m_badContainers(badContainers),
    m_verbose(verbose)
{ }

ResolveClassesVisitor::~ResolveClassesVisitor() {
  if (m_verbose) {
    for (const auto& name : m_needsScanMethod) {
      warning("Can't resolve decl for gc class '%s'", name);
    }
    for (const auto& name : m_gcContainerNames) {
      warning("Can't resolve decl for gc container '%s'", name);
    }
    for (const auto& name : m_hasScanMethodNames) {
      warning("Can't resolve decl for has scan method class '%s'", name);
    }
    for (const auto& name : m_ignoredNames) {
      warning("Can't resolve decl for has ignored class '%s'", name);
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
    if (m_verbose) {
      std::cout << getName(decl) << "(" << decl << ") needs scan.\n";
    }
  }

  if (!exists(m_gcContainers, decl) && exists(m_gcContainerNames, name)) {
    m_gcContainers.insert(decl);
    m_gcContainerNames.erase(m_gcContainerNames.find(name));
    if (m_verbose) {
      std::cout << getName(decl) << "(" << decl << ") needs scan.\n";
    }
  }

  if (!exists(m_ignoredClasses, decl) && exists(m_ignoredNames, name)) {
    m_ignoredClasses.insert(decl);
    m_ignoredNames.erase(m_ignoredNames.find(name));
    if (m_verbose) {
      std::cout << getName(decl) << "(" << decl << ") is ignored.\n";
    }
  }
  if (!exists(m_badContainers, decl) && exists(m_badContainerNames, name)) {
    m_badContainers.insert(decl);
    m_badContainerNames.erase(m_badContainerNames.find(name));
    if (m_verbose) {
      std::cout << getName(decl) << "(" << decl << ") bad container.\n";
    }
  }
  if (!exists(m_hasScanMethod, decl) && exists(m_hasScanMethodNames, name)) {
    m_hasScanMethod.insert(decl);
    m_hasScanMethodNames.erase(m_hasScanMethodNames.find(name));
    if (m_verbose) {
      std::cout << getName(decl) << "(" << decl << ") has scan method.\n";
    }
  }
}

bool ResolveClassesVisitor::VisitFieldDecl(FieldDecl* decl) {
  auto record = decl->getType()->getAsCXXRecordDecl();
  if (record && !isAnonymous(record)) {
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

bool ResolveClassesVisitor::VisitClassTemplateSpecializationDecl(
  ClassTemplateSpecializationDecl* tdecl
) {
  if (tdecl->isThisDeclarationADefinition()) {
    resolveDecl(tdecl->getSpecializedTemplate()->getTemplatedDecl());
  }
  return true;
}

bool ResolveClassesVisitor::VisitCXXRecordDecl(CXXRecordDecl* decl) {
  if (!isAnonymous(decl)) {
    resolveDecl(decl);
  }
  return true;
}

}
