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

#ifndef incl_HPHP_CLANG_GC_TOOL_SCAN_GENERATOR_H
#define incl_HPHP_CLANG_GC_TOOL_SCAN_GENERATOR_H

#undef __GXX_RTTI

#include <set>
#include <map>
#include <iosfwd>
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "hphp/tools/clang-gc-tool/plugin-util.h"

namespace HPHP {

struct ScanGenerator : clang::RecursiveASTVisitor<ScanGenerator>,
                       private PluginUtil {
  ScanGenerator(
    clang::ASTContext& context,
    clang::Rewriter& rewriter,
    DeclSet& hasScanMethod,
    const DeclSet& badContainers,
    DeclSet& gcClasses,
    const std::string& outdir,
    bool verbose
  );

  ~ScanGenerator();

  void preVisit();
  void emitScanMethods();

  bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
  bool VisitFieldDecl(clang::FieldDecl* decl);
  bool VisitClassTemplateDecl(clang::ClassTemplateDecl* tdecl);
  bool VisitDeclRefExpr(clang::DeclRefExpr* declref);

 private:
  void visitBaseClasses(const clang::CXXRecordDecl* decl);
  bool visitCXXRecordDeclCommon(const clang::CXXRecordDecl* udecl,
                                bool forceScan = false);
  bool isResourceOrObject(const clang::CXXRecordDecl* def) const;
  bool isInDeclClass(const DeclSet& decls,
                     const clang::CXXRecordDecl* decl) const;
  bool isGcClass(const clang::CXXRecordDecl* decl) const;
  bool isGcClass(const clang::Type& ty) const;
  bool hasScanMethod(const clang::CXXRecordDecl* decl) const;
  bool hasScanMethod(const clang::Type& ty) const;
  bool isBadContainer(const clang::CXXRecordDecl* decl) const;
  bool isBadContainer(const clang::Type& ty) const;
  void generateWarnings(const clang::CXXRecordDecl* decl);

  enum class NeedsScanFlag { No, Maybe, Yes };

  static const char* toStr(NeedsScanFlag);

  // use visitor in here too.  make a closure of all types/decls that need gc
  NeedsScanFlag needsScanMethod(const clang::Type& ty);
  NeedsScanFlag needsScanMethod(const clang::CXXRecordDecl* decl);

  void dumpFriender(std::ostream& os,
                    const clang::CXXRecordDecl* def,
                    const clang::FieldDecl* field) const;

  bool cloneDef(std::ostream& os,
                std::ostream& friends,
                const clang::CXXRecordDecl* def);
  void declareClass(std::ostream& os,
                    const clang::CXXRecordDecl* def,
                    bool do_close = false) const;
  void declareClonedClass(std::ostream& os,
                          const clang::CXXRecordDecl* def,
                          bool do_close = false) const;
  std::string maybeCloneName(std::string str) const;

  // Add scan method to class.
  void addScanMethod(std::ostream& os,
                     std::ostream& friends,
                     const clang::CXXRecordDecl* def);
  void dumpScanMethodDecl(std::ostream& out,
                          const clang::CXXRecordDecl* def,
                          bool declareArgs = false) const;
  void dumpFieldMarks(std::ostream& out,
                      std::ostream& friends,
                      const clang::CXXRecordDecl* def,
                      bool isCloned);

  void emitProlog(std::ostream& os,
                  const DeclSet& decls) const;
  void emitEpilog(std::ostream& os) const;
  void emitDeclProlog(std::ostream& os) const;
  void emitDeclEpilog(std::ostream& os) const;
  void emitEpilogs();
  void writeFiles();

  void dumpTemplateDecl(std::ostream& out,
                        const char* first,
                        const clang::CXXRecordDecl* def) const;
  void dumpTemplateArgs(std::ostream& out,
                        const char* first,
                        const clang::CXXRecordDecl* def) const;

  std::string getScanFilename(const std::string& file) const;
  void storeScanOutput(const clang::NamedDecl* decl,
                       std::stringstream& scanners,
                       std::stringstream& friends);

  // Find if field decl is tagged with a HHVM_NEEDS_USER_SCAN_METHOD comment.
  // Return empty StringRef if it is not tagged, otherwise the StringRef will
  // point at the custom scan method text.
  clang::StringRef findUserMarkMethod(const clang::FieldDecl* field) const;

  clang::Rewriter& m_rewriter;
  DeclSet& m_hasScanMethodSet;
  const DeclSet& m_badContainers;
  DeclSet& m_gcClasses;
  DeclSet m_checked;
  DeclSet m_visited;
  DeclSet m_privateDefs;
  DeclMap m_whys;
  std::map<std::string, std::stringstream> m_scanMethods;
  std::map<std::string, std::stringstream> m_scanFriends;
  std::map<std::string, DeclSet> m_scanDecls;
  std::string m_outdir;
  bool m_verbose{false};
  const clang::NamedDecl* m_resourceDecl{nullptr};
  const clang::NamedDecl* m_objectDecl{nullptr};
  std::set<std::string> m_clonedNames;
  DeclSet m_sawDefinition;
};

}

#endif
