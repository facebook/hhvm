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

#ifndef incl_HPHP_CLANG_GC_TOOL_RESOLVE_CLASSES_H
#define incl_HPHP_CLANG_GC_TOOL_RESOLVE_CLASSES_H

#include <set>
#include "clang/AST/RecursiveASTVisitor.h"
#include "hphp/tools/clang-gc-tool/plugin-util.h"

namespace HPHP {

/*
 * A pass that finds all the canonical decls for symbol names.  The
 * canonical decls are used in the scan generation pass as set keys.
 *
 */
struct ResolveClassesVisitor
    : clang::RecursiveASTVisitor<ResolveClassesVisitor>,
      private PluginUtil {
  ResolveClassesVisitor(
    clang::ASTContext& context,
    const std::set<std::string>& needsScanMethod,
    DeclSet& gcClasses,
    const std::set<std::string>& hasScanMethodNames,
    DeclSet& hasScanMethod,
    const std::set<std::string>& badContainerNames,
    DeclSet& badContainers,
    bool verbose
  );

  ~ResolveClassesVisitor();

  bool VisitClassTemplateDecl(clang::ClassTemplateDecl* decl);
  bool VisitFieldDecl(clang::FieldDecl* decl);
  bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
  //bool VisitCallExpr(clang::CallExpr* call);
 private:
  void resolveDecl(const clang::CXXRecordDecl* decl);

  std::set<std::string> m_needsScanMethod;
  DeclSet& m_gcClasses;
  std::set<std::string> m_hasScanMethodNames;
  DeclSet& m_hasScanMethod;
  std::set<std::string> m_badContainerNames;
  DeclSet& m_badContainers;
  bool m_verbose;
};

}

#endif
