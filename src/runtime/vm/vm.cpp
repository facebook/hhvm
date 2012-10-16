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

#include <runtime/base/runtime_option.h>
#include <runtime/base/execution_context.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/vm.h>
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
#ifndef HHVM
const long long hhbc_ext_funcs_count = 0;
const HhbcExtFuncInfo hhbc_ext_funcs[] = {};
const long long hhbc_ext_class_count = 0;
const HhbcExtClassInfo hhbc_ext_classes[] = {};
#endif
namespace VM {

///////////////////////////////////////////////////////////////////////////////

static StaticString s_stdclass(LITSTR_INIT("stdclass"));
static StaticString s_Exception(LITSTR_INIT("Exception"));
static StaticString s_BadMethodCallException(
  LITSTR_INIT("BadMethodCallException"));
static StaticString s_InvalidArgumentException(
  LITSTR_INIT("InvalidArgumentException"));
static StaticString s_RuntimeException(LITSTR_INIT("RuntimeException"));
static StaticString s_OutOfBoundsException(
  LITSTR_INIT("OutOfBoundsException"));
static StaticString s_InvalidOperationException(
  LITSTR_INIT("InvalidOperationException"));
static StaticString s_Directory(LITSTR_INIT("Directory"));
static StaticString s_RecursiveDirectoryIterator(
  LITSTR_INIT("RecursiveDirectoryIterator"));
static StaticString s_SplFileInfo(LITSTR_INIT("SplFileInfo"));
static StaticString s_SplFileObject(LITSTR_INIT("SplFileObject"));
static StaticString s_pinitSentinel(LITSTR_INIT("__pinitSentinel"));
static StaticString s_resource(LITSTR_INIT("__resource"));
static StaticString s_DOMException(LITSTR_INIT("DOMException"));
static StaticString s_PDOException(LITSTR_INIT("PDOException"));
static StaticString s_SoapFault(LITSTR_INIT("SoapFault"));
static StaticString s_Continuation(LITSTR_INIT("Continuation"));

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
    if ((ci = g_vmContext->findClassInfo(name)) != NULL
        || (ci = g_vmContext->findInterfaceInfo(name)) != NULL
        || (ci = g_vmContext->findTraitInfo(name)) != NULL) {
      return ci;
    }
    return NULL;
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
  // Initialize compiler state
  VM::compile_file(0, 0, MD5(), 0);

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
  bool p = RuntimeOption::RepoAuthoritative;
  bool rp = RuntimeOption::AlwaysUseRelativePath;
  bool sf = RuntimeOption::SafeFileAccess;
  RuntimeOption::EvalDumpBytecode = false;
  RuntimeOption::RepoAuthoritative = false;
  RuntimeOption::AlwaysUseRelativePath = false;
  RuntimeOption::SafeFileAccess = false;

  Transl::TargetCache::requestInit();

  Unit* nativeFuncUnit = build_native_func_unit(hhbc_ext_funcs,
                                                hhbc_ext_funcs_count);
  SystemLib::s_nativeFuncUnit = nativeFuncUnit;

  // Search for systemlib.php in the following places:
  // 1) ${HHVM_LIB_PATH}/systemlib.php
  // 2) <dirname(realpath(hhvm))>/systemlib.php (requires proc filesystem)
  // 3) ${HPHP_LIB}/systemlib.php
  // 4) <HHVM_LIB_PATH_DEFAULT>/systemlib.php
  //
  // HHVM_LIB_PATH allows a manual override at runtime. If systemlib.php
  // exists next to the hhvm binary, that is likely to be the next best
  // version to use. The realpath()-based lookup will succeed as long as the
  // proc filesystem exists (e.g. on Linux and some FreeBSD configurations)
  // and no hard links are in use for the executable. Under certain build
  // situations, systemlib.php will not be generated next to hhvm binary, so
  // ${HPHP_LIB} is checked next. Failing all of those options, the
  // HHVM_LIB_PATH_DEFAULT-based lookup will always succeed, assuming that the
  // application was built and installed correctly.
  String currentDir = g_vmContext->getCwd();
  HPHP::Eval::PhpFile* file = NULL;

#define SYSTEMLIB_PHP "/systemlib.php"
#define LOOKUP_STR(s) do {                                                    \
  String systemlibPath = String(s) + SYSTEMLIB_PHP;                           \
  file = g_vmContext->lookupPhpFile(systemlibPath.get(), currentDir.data(),   \
                                    NULL);                                    \
} while (0)
#define LOOKUP_ENV(v) do {                                                    \
  if (!file) {                                                                \
    const char* s = getenv(#v);                                               \
    if (s && *s) {                                                            \
      LOOKUP_STR(s);                                                          \
    }                                                                         \
  }                                                                           \
} while (0)
#define LOOKUP_CPP(v) do {                                                    \
  if (!file) {                                                                \
    LOOKUP_STR(v);                                                            \
  }                                                                           \
} while (0)

  LOOKUP_ENV(HHVM_LIB_PATH);
  if (!file) {
    char hhvm_exe[PATH_MAX+1];
    char hhvm_path[PATH_MAX+1];
    ssize_t len = readlink("/proc/self/exe", hhvm_exe, sizeof(hhvm_exe));
    if (len >= 0) {
      hhvm_exe[len] = '\0';
      if (realpath(hhvm_exe, hhvm_path) != NULL) {
        char *hphp_lib = dirname(hhvm_path);
        LOOKUP_STR(hphp_lib);
      }
    }
  }
  LOOKUP_ENV(HPHP_LIB);
#ifdef HHVM_LIB_PATH_DEFAULT
  LOOKUP_CPP(HHVM_LIB_PATH_DEFAULT);
#endif
  if (!file) {
    // Die a horrible death.
    Logger::Error("Unable to find/load systemlib.php");
    _exit(1);
  }
#undef SYSTEMLIB_PHP
#undef LOOKUP_STR
#undef LOOKUP_ENV
#undef LOOKUP_CPP
  SystemLib::s_phpFile = file;
  file->incRef();
  SystemLib::s_unit = file->unit();

  // Restore most settings before merging anything,
  // because of optimizations that depend on the
  // setting of RepoAuthoritative
  RuntimeOption::RepoAuthoritative = p;
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
    Class *cls = *Unit::GetNamedEntity(s_##cls.get())->clsList();       \
    ASSERT(!hhbc_ext_class_count || cls);                               \
    SystemLib::s_##cls##Class = cls;                                    \
  }

  // Stash a pointer to the VM Classes for stdclass, Exception,
  // pinitSentinel and resource
  INIT_SYSTEMLIB_CLASS_FIELD(stdclass);
  INIT_SYSTEMLIB_CLASS_FIELD(Exception);
  INIT_SYSTEMLIB_CLASS_FIELD(BadMethodCallException);
  INIT_SYSTEMLIB_CLASS_FIELD(InvalidArgumentException);
  INIT_SYSTEMLIB_CLASS_FIELD(RuntimeException);
  INIT_SYSTEMLIB_CLASS_FIELD(OutOfBoundsException);
  INIT_SYSTEMLIB_CLASS_FIELD(InvalidOperationException);
  INIT_SYSTEMLIB_CLASS_FIELD(Directory);
  INIT_SYSTEMLIB_CLASS_FIELD(RecursiveDirectoryIterator);
  INIT_SYSTEMLIB_CLASS_FIELD(SplFileInfo);
  INIT_SYSTEMLIB_CLASS_FIELD(SplFileObject);
  INIT_SYSTEMLIB_CLASS_FIELD(pinitSentinel);
  INIT_SYSTEMLIB_CLASS_FIELD(resource);
  INIT_SYSTEMLIB_CLASS_FIELD(DOMException);
  INIT_SYSTEMLIB_CLASS_FIELD(PDOException);
  INIT_SYSTEMLIB_CLASS_FIELD(SoapFault);
  INIT_SYSTEMLIB_CLASS_FIELD(Continuation);

#undef INIT_SYSTEMLIB_CLASS_FIELD

  // Retrieve all of the class pointers
  for (long long i = 0LL; i < hhbc_ext_class_count; ++i) {
    const HhbcExtClassInfo* info = hhbc_ext_classes + i;
    const StringData* name = StringData::GetStaticString(info->m_name);
    const NamedEntity* ne = Unit::GetNamedEntity(name);
    Class* cls = Unit::lookupClass(ne);
    ASSERT(cls);
    const ObjectStaticCallbacks* osc =
      get_object_static_callbacks(info->m_name);
    ASSERT(osc != NULL);
    *(osc->os_cls_ptr) = cls;
  }

  Stack::ValidateStackSize();
  SystemLib::s_inited = true;

  // For debug build, run some quick unit tests at process start time
  if (debug) {
    VM::Transl::FixupMapUnitTest _;
  }

  // Restore this after loading systemlib
  RuntimeOption::EvalDumpBytecode = db;
}

///////////////////////////////////////////////////////////////////////////////
}
}
