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

#include <compiler/analysis/emitter.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/execution_context.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/funcdict.h>
#include <runtime/vm/runtime.h>
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/vm/translator/fixup.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/eval/runtime/file_repository.h>
#include <system/lib/systemlib.h>
#include <util/logger.h>

#include <libgen.h> // For dirname(3).
#include <string>

namespace HPHP {
namespace VM {

///////////////////////////////////////////////////////////////////////////////

#define STRINGIZE_CLASS_NAME(cls) #cls
#define pinitSentinel __pinitSentinel
#define resource __resource

#define SYSTEM_CLASS_STRING(cls)                        \
  static StaticString s_##cls(LITSTR_INIT(STRINGIZE_CLASS_NAME(cls)));
SYSTEMLIB_CLASSES(SYSTEM_CLASS_STRING)

#undef resource
#undef pinitSentinel
#undef STRINGIZE_CLASS_NAME

class VMClassInfoHook : public ClassInfoHook {
public:
  virtual Array getUserFunctions() const {
    return g_vmContext->getUserFunctionsInfo();
  }
  virtual Array getClasses() const {
    return Unit::getClassesInfo();
  }
  virtual Array getInterfaces() const {
    return Unit::getInterfacesInfo();
  }
  virtual Array getTraits() const {
    return Unit::getTraitsInfo();
  }
  virtual Array getConstants() const {
    return g_vmContext->getConstantsInfo();
  }
  virtual const ClassInfo::MethodInfo *findFunction(CStrRef name) const {
    return g_vmContext->findFunctionInfo(name);
  }
  virtual const ClassInfo *findClassLike(CStrRef name) const {
    const ClassInfo* ci;
    if ((ci = g_vmContext->findClassInfo(name)) != nullptr
        || (ci = g_vmContext->findInterfaceInfo(name)) != nullptr
        || (ci = g_vmContext->findTraitInfo(name)) != nullptr) {
      return ci;
    }
    return nullptr;
  }
  virtual const ClassInfo *findInterface(CStrRef name) const {
    return g_vmContext->findInterfaceInfo(name);
  }
  virtual const ClassInfo* findTrait(CStrRef name) const {
    return g_vmContext->findTraitInfo(name);
  }
  virtual const ClassInfo::ConstantInfo *findConstant(CStrRef name) const {
    return g_vmContext->findConstantInfo(name);
  }
};

static VMClassInfoHook vm_class_info_hook;

void ProcessInit() {
  // Install VM's ClassInfoHook
  ClassInfo::SetHook(&vm_class_info_hook);

  // ensure that nextTx64 and tx64 are set
  (void)VM::Transl::Translator::Get();

  if (!RuntimeOption::RepoAuthoritative &&
      RuntimeOption::EvalJitEnableRenameFunction &&
      RuntimeOption::EvalJit) {
    VM::Func::enableIntercept();
    VM::Transl::TranslatorX64* tx64 = VM::Transl::TranslatorX64::Get();
    tx64->enableIntercepts();
  }
  // Save the current options, and set things up so that
  // systemlib.php can be read from and stored in the
  // normal repo.
  bool db = RuntimeOption::EvalDumpBytecode;
  bool rp = RuntimeOption::AlwaysUseRelativePath;
  bool sf = RuntimeOption::SafeFileAccess;
  RuntimeOption::EvalDumpBytecode = false;
  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;

  Transl::TargetCache::requestInit();

  Unit* nativeFuncUnit = build_native_func_unit(hhbc_ext_funcs,
                                                hhbc_ext_funcs_count);
  SystemLib::s_nativeFuncUnit = nativeFuncUnit;

  // Search for systemlib.php in the following places:
  // 1) ${HHVM_LIB_PATH}/systemlib.php
  // 2) <dirname(realpath(hhvm))>/systemlib.php (requires proc filesystem)
  // 3) <HHVM_LIB_PATH_DEFAULT>/systemlib.php
  //
  // HHVM_LIB_PATH allows a manual override at runtime. If systemlib.php
  // exists next to the hhvm binary, that is likely to be the next best
  // version to use. The realpath()-based lookup will succeed as long as the
  // proc filesystem exists (e.g. on Linux and some FreeBSD configurations)
  // and no hard links are in use for the executable. Failing all of those
  // options, the HHVM_LIB_PATH_DEFAULT-based lookup will always succeed,
  // assuming that the application was built and installed correctly.
  String currentDir = g_vmContext->getCwd();
  HPHP::Eval::PhpFile* file = nullptr;

  string slib = RuntimeOption::RepoAuthoritative ?
    string("/:systemlib.php") : systemlib_path();

  if (!slib.empty()) {
    file = g_vmContext->lookupPhpFile(String(slib).get(),
                                      currentDir.data(), nullptr);
  }
  if (!file) {
    // Die a horrible death.
    Logger::Error("Unable to find/load systemlib.php");
    _exit(1);
  }

  SystemLib::s_phpFile = file;
  file->incRef();
  SystemLib::s_unit = file->unit();

  // Restore most settings before merging anything,
  // because of optimizations that depend on them
  RuntimeOption::AlwaysUseRelativePath = rp;
  RuntimeOption::SafeFileAccess = sf;

  // Load the systemlib unit to build the Class objects
  SystemLib::s_unit->merge();

  // load builtins
  SystemLib::s_nativeFuncUnit->merge();

  // We call a special bytecode emitter function to build the native
  // unit which will contain all of our cppext functions and classes.
  // Each function and method will have a bytecode body that will thunk
  // to the native implementation.
  Unit* nativeClassUnit = build_native_class_unit(hhbc_ext_classes,
                                                  hhbc_ext_class_count);
  SystemLib::s_nativeClassUnit = nativeClassUnit;

  // Load the nativelib unit to build the Class objects
  SystemLib::s_nativeClassUnit->merge();

#define INIT_SYSTEMLIB_CLASS_FIELD(cls)                                 \
  {                                                                     \
    Class *cls = Unit::GetNamedEntity(s_##cls.get())->clsList();       \
    assert(!hhbc_ext_class_count || cls);                               \
    SystemLib::s_##cls##Class = cls;                                    \
  }

  // Stash a pointer to the VM Classes for stdclass, Exception,
  // pinitSentinel and resource
  SYSTEMLIB_CLASSES(INIT_SYSTEMLIB_CLASS_FIELD)

#undef INIT_SYSTEMLIB_CLASS_FIELD

  // Retrieve all of the class pointers
  for (long long i = 0; i < hhbc_ext_class_count; ++i) {
    const HhbcExtClassInfo* info = hhbc_ext_classes + i;
    const StringData* name = StringData::GetStaticString(info->m_name);
    const NamedEntity* ne = Unit::GetNamedEntity(name);
    Class* cls = Unit::lookupClass(ne);
    assert(cls);
    *(info->m_clsPtr) = cls;
  }

  Stack::ValidateStackSize();
  SystemLib::s_inited = true;

  // Restore last to avoid dumping system things
  RuntimeOption::EvalDumpBytecode = db;
}

///////////////////////////////////////////////////////////////////////////////
}
}
