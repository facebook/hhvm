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
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/ASTConsumer.h>

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
    const std::set<std::string>& gcContainerNames,
    const std::set<std::string>& hasScanMethodNames,
    const std::set<std::string>& ignoredNames,
    const std::set<std::string>& badContainerNames,
    const std::string& outdir,
    bool verbose
  ) : m_rewriter(mgr, opts),
      m_resolveVisitor(context,
                       needsScanMethodNames,
                       m_gcClasses,
                       gcContainerNames,
                       m_gcContainers,
                       hasScanMethodNames,
                       m_hasScanMethod,
                       ignoredNames,
                       m_ignoredClasses,
                       badContainerNames,
                       m_badContainers,
                       verbose),
      m_generator(context,
                  m_rewriter,
                  m_hasScanMethod,
                  m_ignoredClasses,
                  m_badContainers,
                  m_gcClasses,
                  m_gcContainers,
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
  DeclSet m_gcContainers;
  DeclSet m_ignoredClasses;
  DeclSet m_hasScanMethod;
  DeclSet m_badContainers;
};

struct AddScanMethodsAction : public PluginASTAction {
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, StringRef) {
    // These are classes that trigger generation of a scan method.
    // Any class that subclasses from or contains one of these classes
    // will also trigger scan method generation.
    m_needsScanMethodNames.insert("HPHP::ObjectData");
    m_needsScanMethodNames.insert("HPHP::ResourceData");
    m_needsScanMethodNames.insert("HPHP::ArrayData");
    m_needsScanMethodNames.insert("HPHP::StringData");
    m_needsScanMethodNames.insert("HPHP::TypedValue");
    m_needsScanMethodNames.insert("HPHP::Cell");
    m_needsScanMethodNames.insert("HPHP::RefData");
    m_needsScanMethodNames.insert("HPHP::Array");
    m_needsScanMethodNames.insert("HPHP::String");
    m_needsScanMethodNames.insert("HPHP::Variant");
    m_needsScanMethodNames.insert("HPHP::Object");
    m_needsScanMethodNames.insert("HPHP::Resource");
    m_needsScanMethodNames.insert("HPHP::RequestEventHandler");
    m_needsScanMethodNames.insert("HPHP::Extension");

    // TODO (t6956600): Add these?
    //m_needsScanMethodNames.insert("HPHP::SweepableMember");

    // These are classes that have scan methods already defined
    // by the heap tracer.  Scan methods will not be generated
    // for any of these classes.
    // Fundamental types are assumed to have scan methods.
    // NeedScanMethod types can also appear in this set.
    // If a particular class causes problems with the scan
    // generator, you may want to include it in this list and
    // make sure there is a hand written scan method.
    // Members of this set should have correpsonding methods
    // on IMarker.
    m_hasScanMethodNames = m_needsScanMethodNames;
    m_hasScanMethodNames.insert("folly::Optional");
    m_hasScanMethodNames.insert("folly::AtomicHashArray");
    m_hasScanMethodNames.insert("folly::AtomicHashMap");
    m_hasScanMethodNames.insert("folly::Singleton");
    m_hasScanMethodNames.insert("folly::SingletonVault");
    m_hasScanMethodNames.insert("folly::detail::SingletonHolder");
    m_hasScanMethodNames.insert("folly::Range");
    m_hasScanMethodNames.insert("HPHP::req::vector");
    m_hasScanMethodNames.insert("HPHP::req::deque");
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
    m_hasScanMethodNames.insert("HPHP::LowPtr");
    m_hasScanMethodNames.insert("HPHP::TlsPodBag");
    m_hasScanMethodNames.insert("HPHP::AtomicSharedPtr");
    m_hasScanMethodNames.insert("HPHP::NameValueTable");
    m_hasScanMethodNames.insert("HPHP::NameValueTable::Elm");
    m_hasScanMethodNames.insert("HPHP::default_ptr");
    m_hasScanMethodNames.insert("HPHP::copy_ptr");
    m_hasScanMethodNames.insert("HPHP::Func");
    m_hasScanMethodNames.insert("HPHP::Unit");
    m_hasScanMethodNames.insert("HPHP::ExtraArgs");
    m_hasScanMethodNames.insert("HPHP::ActRec");
    m_hasScanMethodNames.insert("HPHP::Stack");
    m_hasScanMethodNames.insert("HPHP::Value");
    m_hasScanMethodNames.insert("HPHP::VarEnv");
    m_hasScanMethodNames.insert("HPHP::ApcTypedValue");
    m_hasScanMethodNames.insert("HPHP::ApcTypedValue::SharedData");
    m_hasScanMethodNames.insert("HPHP::MixedArray");
    m_hasScanMethodNames.insert("HPHP::MixedArray::Elm");
    m_hasScanMethodNames.insert("HPHP::MixedArray::ValIter");
    m_hasScanMethodNames.insert("HPHP::HashCollection");
    m_hasScanMethodNames.insert("HPHP::ArrayIter");
    m_hasScanMethodNames.insert("HPHP::MArrayIter");
    m_hasScanMethodNames.insert("HPHP::Iter");
    m_hasScanMethodNames.insert("HPHP::Iter::Data");
    m_hasScanMethodNames.insert("HPHP::TypedValueAux");
    m_hasScanMethodNames.insert("HPHP::hphp_hash_map");
    m_hasScanMethodNames.insert("HPHP::hphp_hash_set");
    m_hasScanMethodNames.insert("HPHP::c_AwaitAllWaitHandle");
    m_hasScanMethodNames.insert("HPHP::ThreadLocal");
    m_hasScanMethodNames.insert("HPHP::ThreadLocalNoCheck");
    m_hasScanMethodNames.insert("HPHP::ThreadLocalProxy");
    m_hasScanMethodNames.insert("HPHP::ThreadLocalSingleton");
    m_hasScanMethodNames.insert("HPHP::WandResource");
    m_hasScanMethodNames.insert("HPHP::IndexedStringMap");
    m_hasScanMethodNames.insert("HPHP::RankedCHM");
    m_hasScanMethodNames.insert("HPHP::TinyVector");
    m_hasScanMethodNames.insert("std::atomic");
    m_hasScanMethodNames.insert("std::pair");
    m_hasScanMethodNames.insert("std::vector");
    m_hasScanMethodNames.insert("std::map");
    m_hasScanMethodNames.insert("std::set");
    m_hasScanMethodNames.insert("std::vector");
    m_hasScanMethodNames.insert("std::deque");
    m_hasScanMethodNames.insert("std::priority_queue");
    m_hasScanMethodNames.insert("std::flat_map");
    m_hasScanMethodNames.insert("std::flat_multimap");
    m_hasScanMethodNames.insert("std::flat_set");
    m_hasScanMethodNames.insert("std::flat_multiset");
    m_hasScanMethodNames.insert("std::stack");
    m_hasScanMethodNames.insert("std::multimap");
    m_hasScanMethodNames.insert("std::set");
    m_hasScanMethodNames.insert("std::multiset");
    m_hasScanMethodNames.insert("std::hash_map");
    m_hasScanMethodNames.insert("std::hash_multimap");
    m_hasScanMethodNames.insert("std::hash_set");
    m_hasScanMethodNames.insert("std::unique_ptr");
    m_hasScanMethodNames.insert("std::shared_ptr");
    m_hasScanMethodNames.insert("std::weak_ptr");
    m_hasScanMethodNames.insert("std::array");
    m_hasScanMethodNames.insert("tbb::concurrent_hash_map");
    m_hasScanMethodNames.insert("tbb::interface5::concurrent_hash_map");
    m_hasScanMethodNames.insert("boost::variant");
    m_hasScanMethodNames.insert("boost::container::flat_map");
    m_hasScanMethodNames.insert("boost::container::vector");
    m_hasScanMethodNames.insert("boost::container::vector::const_iterator");

    // These classes are templates that hide their uses of scanable objects
    // with casts or other tricks.  In order to recognized these as important
    // classes, we instead check if any of the template parameters contain
    // scanable types.
    m_gcContainerNames.insert("HPHP::req::flat_map");
    m_gcContainerNames.insert("HPHP::req::flat_multimap");
    m_gcContainerNames.insert("HPHP::req::flat_set");
    m_gcContainerNames.insert("HPHP::req::flat_multiset");
    m_gcContainerNames.insert("HPHP::req::hash_map");
    m_gcContainerNames.insert("HPHP::req::hash_multimap");
    m_gcContainerNames.insert("HPHP::req::hash_set");
    m_gcContainerNames.insert("HPHP::hphp_hash_map");
    m_gcContainerNames.insert("HPHP::hphp_hash_set");
    m_gcContainerNames.insert("std::flat_map");
    m_gcContainerNames.insert("std::flat_multimap");
    m_gcContainerNames.insert("std::flat_set");
    m_gcContainerNames.insert("std::flat_multiset");
    m_gcContainerNames.insert("std::hash_map");
    m_gcContainerNames.insert("std::hash_multimap");
    m_gcContainerNames.insert("std::hash_set");
    m_gcContainerNames.insert("std::unordered_map");
    m_gcContainerNames.insert("std::unordered_set");

    // These classes are ignored during analysis.  They are either
    // here because they should not be scanned or because they
    // cause problems with scan code generation.
    // TODO (t6956600) This list should be double-checked.

    // Part of memory manager.
    m_ignoredNames.insert("HPHP::Header");
    m_ignoredNames.insert("HPHP::MemoryManager");

    // Ignored.
    m_ignoredNames.insert("HPHP::AsyncFunc");

    // Test code
    m_ignoredNames.insert("HPHP::DummyResource2");
    m_ignoredNames.insert("HPHP::_php_ezctest_obj");
    m_ignoredNames.insert("HPHP::_zend_ezc_test_globals");
    m_ignoredNames.insert("_php_ezctest_obj");
    m_ignoredNames.insert("_zend_ezc_test_globals");
    m_ignoredNames.insert("HPHP::TestTransport");

    // Opaque third party types.
    m_ignoredNames.insert("MagickWand");
    m_ignoredNames.insert("PixelWand");
    m_ignoredNames.insert("DrawingWand");
    m_ignoredNames.insert("PixelIterator");

    // Module dependency problems
    m_ignoredNames.insert("HPHP::DnsEvent");
    m_ignoredNames.insert("HPHP::GatehouseRequestEventHandler");
    m_ignoredNames.insert("HPHP::ProxygenTransport");
    m_ignoredNames.insert("HPHP::CacheClientEvent");
    m_ignoredNames.insert("HPHP::FastCGIServer");
    m_ignoredNames.insert("HPHP::FastCGIWorker");
    m_ignoredNames.insert("HPHP::FastCGISession");
    m_ignoredNames.insert("HPHP::FastCGITransport");

    // Static arrays and strings from HHBBC.
    m_ignoredNames.insert("HPHP::SString");
    m_ignoredNames.insert("HPHP::SArray");

    // Compilation problems.  TODO (t6956600): Whittle these down.

    // Name used in template is not properly getting qualifiers, e.g.
    // const std::enable_shared_from_this<DebuggerProxy>.
    m_ignoredNames.insert("HPHP::Eval::DebuggerProxy");

    // A private class with a nested typedef used in a templatized map, e.g.
    // AtomicHashArray<..., HPHP::PCRECache::ahm_string_data_same, ...>.
    m_ignoredNames.insert("HPHP::PCRECache");

    // Causes duplicate definition of private cloned classes, e.g. see
    // licm.cpp: Env, refcount-opts.cpp: Env.
    m_ignoredNames.insert("HPHP::jit::AliasAnalysis");

    // Causes downstream problems, e.g. gvn.cpp.  Instantiation of
    // template that uses private classes as template args. Similar to
    // PCRECache.
    m_ignoredNames.insert("HPHP::jit::Type");

    // Causes downstream problems.
    m_ignoredNames.insert("HPHP::jit::SSATmp");
    m_ignoredNames.insert("HPHP::jit::StateVector");
    m_ignoredNames.insert("HPHP::jit::NormalizedInstruction");
    m_ignoredNames.insert("HPHP::jit::IRUnit");

    /////////////////////////

    // These are classes that use malloc/new allocation internally.
    // They should generally not be used to store scanable things.
    // This list is used to generate warnings.
    m_badContainerNames.insert("std::vector");
    m_badContainerNames.insert("std::map");
    m_badContainerNames.insert("std::set");
    m_badContainerNames.insert("std::vector");
    m_badContainerNames.insert("std::deque");
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
    m_badContainerNames.insert("std::array");
    m_badContainerNames.insert("boost::container::flat_map");
    m_badContainerNames.insert("boost::container::flat_set");
    m_badContainerNames.insert("boost::container::set");
    m_badContainerNames.insert("boost::container::map");
    m_badContainerNames.insert("boost::container::vector");
    // etc.

    return std::unique_ptr<ASTConsumer>(new AddScanMethodsConsumer(
                                          CI.getASTContext(),
                                          CI.getSourceManager(),
                                          CI.getLangOpts(),
                                          m_needsScanMethodNames,
                                          m_gcContainerNames,
                                          m_hasScanMethodNames,
                                          m_ignoredNames,
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
  std::set<std::string> m_gcContainerNames;
  std::set<std::string> m_hasScanMethodNames;
  std::set<std::string> m_ignoredNames;
  std::set<std::string> m_badContainerNames;
};

static FrontendPluginRegistry::Add<AddScanMethodsAction>
X("add-scan-methods", "Add GC scan methods to tagged classes");

}
