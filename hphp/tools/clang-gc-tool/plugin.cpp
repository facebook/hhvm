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

/*
 * This is the Clang plugin registration and driver code.  It sets up the
 * different passes (ResolveClasses, ScanGenerator) and maintains the white
 * lists of different classes.
 *
 * needsScanMethodNames are names of classes that require a scan method.  Any
 * class inheriting from one of these classes will also be marked as needing
 * a scan method.
 *
 * hasScanMethodNames are names of classes that should have user written scan
 * functions.  These classes may overlap with needScanMethodNames classes.
 *
 * badContainerNames are names of container classes that are not suitable for
 * holding request-allocated objects, e.g. std::vector.
 *
 * HandleTranslationUnit is the method that runs the different passes.  It is
 * invoked by Clang once any source files have finished parsing.
 *
 */

#undef __GXX_RTTI

#include <iostream>
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"

#include "hphp/tools/clang-gc-tool/plugin-util.h"
#include "hphp/tools/clang-gc-tool/resolve-classes.h"
#include "hphp/tools/clang-gc-tool/scan-generator.h"

namespace HPHP {

using clang::ASTContext;
using clang::ASTConsumer;
using clang::SourceManager;
using clang::Rewriter;
using clang::LangOptions;
using clang::PluginASTAction;
using clang::CompilerInstance;
using clang::StringRef;
using clang::FrontendPluginRegistry;

struct AddScanMethodsConsumer : public ASTConsumer {
  AddScanMethodsConsumer(
    ASTContext& context,
    SourceManager& mgr,
    const LangOptions& opts,
    const std::set<std::string>& needsScanMethodNames,
    const std::set<std::string>& hasScanMethodNames,
    const std::set<std::string>& badContainerNames,
    const std::string& outdir,
    bool verbose
  ) : m_rewriter(mgr, opts),
      m_resolveVisitor(context,
                       needsScanMethodNames,
                       m_gcClasses,
                       hasScanMethodNames,
                       m_hasScanMethod,
                       badContainerNames,
                       m_badContainers,
                       verbose),
      m_generator(context,
                  m_rewriter,
                  m_hasScanMethod,
                  m_badContainers,
                  m_gcClasses,
                  outdir,
                  verbose)
  {}

  virtual void HandleTranslationUnit(ASTContext& context) {
    // Resolve all string class names to clang NamedDecls.
    m_resolveVisitor.TraverseDecl(context.getTranslationUnitDecl());

    // Visit all declarations.
    m_generator.preVisit();
    m_generator.TraverseDecl(context.getTranslationUnitDecl());
    m_generator.emitScanMethods();
  }
 private:
  Rewriter m_rewriter;
  ResolveClassesVisitor m_resolveVisitor;
  ScanGenerator m_generator;

  DeclSet m_gcClasses;
  DeclSet m_hasScanMethod;
  DeclSet m_badContainers;
};

struct AddScanMethodsAction : public PluginASTAction {
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, StringRef) {
    // These are classes that trigger generation of a scan method.
    // Any class that subclasses from one of these classes will
    // also trigger scan method generation.
    m_needsScanMethodNames.insert("HPHP::ObjectData");
    m_needsScanMethodNames.insert("HPHP::ResourceData");
    m_needsScanMethodNames.insert("HPHP::ArrayData");
    m_needsScanMethodNames.insert("HPHP::StringData");
    m_needsScanMethodNames.insert("HPHP::TypedValue");
    m_needsScanMethodNames.insert("HPHP::Cell");
    m_needsScanMethodNames.insert("HPHP::RefData");
    m_needsScanMethodNames.insert("HPHP::NameValueTable");
    m_needsScanMethodNames.insert("HPHP::Array");
    m_needsScanMethodNames.insert("HPHP::String");
    m_needsScanMethodNames.insert("HPHP::Variant");
    m_needsScanMethodNames.insert("HPHP::Object");
    m_needsScanMethodNames.insert("HPHP::Resource");
    m_needsScanMethodNames.insert("HPHP::req::ptr");
    m_needsScanMethodNames.insert("HPHP::LowPtr");
    //m_needsScanMethodNames.insert("HPHP::SweepableMember");
    //m_needsScanMethodNames.insert("HPHP::RequestEventHandler");
    //m_needsScanMethodNames.insert("HPHP::Extension");

    // These are classes that have scan methods already defined
    // by the heap tracer and are ignored when scan generation
    // happens.
    // Fundamental types are assumed to have scan methods.
    // NeedScanMethod types can also appear in this set.
    // If a particular class causes problems with the scan
    // generator, you may want to include it in this list and
    // make sure there is a hand written scan method.
    m_hasScanMethodNames.insert("HPHP::req::vector");
    m_hasScanMethodNames.insert("HPHP::req::dequeue");
    m_hasScanMethodNames.insert("HPHP::req::priority_queue");
    m_hasScanMethodNames.insert("HPHP::req::flat_map");
    m_hasScanMethodNames.insert("HPHP::req::flat_multimap");
    m_hasScanMethodNames.insert("HPHP::req::flat_set");
    m_hasScanMethodNames.insert("HPHP::req::flat_multiset");
    m_hasScanMethodNames.insert("HPHP::req::stack");
    m_hasScanMethodNames.insert("HPHP::req::map");
    m_hasScanMethodNames.insert("HPHP::req::multimap");
    m_hasScanMethodNames.insert("HPHP::req::set");
    m_hasScanMethodNames.insert("HPHP::req::multiset");
    m_hasScanMethodNames.insert("HPHP::req::hash_map");
    m_hasScanMethodNames.insert("HPHP::req::hash_multimap");
    m_hasScanMethodNames.insert("HPHP::req::hash_set");
    m_hasScanMethodNames.insert("HPHP::req::unique_ptr");
    m_hasScanMethodNames.insert("HPHP::req::ptr");
    m_hasScanMethodNames.insert("HPHP::AtomicSmartPtr");
    m_hasScanMethodNames.insert("HPHP::AtomicSharedPtr");
    m_hasScanMethodNames.insert("HPHP::SString");
    m_hasScanMethodNames.insert("HPHP::SArray");
    m_hasScanMethodNames.insert("HPHP::Array");
    m_hasScanMethodNames.insert("HPHP::String");
    m_hasScanMethodNames.insert("HPHP::Variant");
    m_hasScanMethodNames.insert("HPHP::Object");
    m_hasScanMethodNames.insert("HPHP::Resource");
    m_hasScanMethodNames.insert("HPHP::ObjectData");  // contains a union
    m_hasScanMethodNames.insert("HPHP::NameValueTable");
    m_hasScanMethodNames.insert("HPHP::NameValueTable::Elm");
    m_hasScanMethodNames.insert("HPHP::DummyResource2"); // from test file
    m_hasScanMethodNames.insert("folly::Optional");
    m_hasScanMethodNames.insert("boost::variant");

    // These are classes that use malloc/new allocation internally.
    // They should generally not be used to store scanable things.
    // This list is used to generate warnings.
    m_badContainerNames.insert("std::vector");
    m_badContainerNames.insert("std::map");
    m_badContainerNames.insert("std::set");
    m_badContainerNames.insert("std::vector");
    m_badContainerNames.insert("std::dequeue");
    m_badContainerNames.insert("std::priority_queue");
    m_badContainerNames.insert("std::flat_map");
    m_badContainerNames.insert("std::flat_multimap");
    m_badContainerNames.insert("std::flat_set");
    m_badContainerNames.insert("std::flat_multiset");
    m_badContainerNames.insert("std::stack");
    m_badContainerNames.insert("std::map");
    m_badContainerNames.insert("std::multimap");
    m_badContainerNames.insert("std::set");
    m_badContainerNames.insert("std::multiset");
    m_badContainerNames.insert("std::hash_map");
    m_badContainerNames.insert("std::hash_multimap");
    m_badContainerNames.insert("std::hash_set");
    m_badContainerNames.insert("std::unique_ptr");
    m_badContainerNames.insert("std::shared_ptr");
    m_badContainerNames.insert("std::weak_ptr");
    // etc.

    return std::unique_ptr<ASTConsumer>(new AddScanMethodsConsumer(
                                          CI.getASTContext(),
                                          CI.getSourceManager(),
                                          CI.getLangOpts(),
                                          m_needsScanMethodNames,
                                          m_hasScanMethodNames,
                                          m_badContainerNames,
                                          m_outdir,
                                          m_verbose));
  }

  bool ParseArgs(
    const CompilerInstance &CI,
    const std::vector<std::string>& args
  ) {
    for (auto itr = args.begin(); itr < args.end(); ++itr) {
      if (*itr == "-scan") {
        if (++itr < args.end()) {
          m_needsScanMethodNames.insert(*itr);
        }
      } else if (*itr == "-v") {
        m_verbose = true;
      } else {
        m_outdir = *itr;
      }
    }
    return true;
  }
  void PrintHelp(llvm::raw_ostream& ros) {
    ros << "-v           verbose.\n";
    ros << "-scan foo    mark class foo as needing a scan method\n.";
    ros << "dirname      directory used to store output.\n";
  }
 private:
  bool m_verbose{false};               // verbose flag
  std::string m_outdir;                // output directory
  std::set<std::string> m_needsScanMethodNames;
  std::set<std::string> m_hasScanMethodNames;
  std::set<std::string> m_badContainerNames;
};

static FrontendPluginRegistry::Add<AddScanMethodsAction>
X("add-scan-methods", "Add GC scan methods to tagged classes");

}
