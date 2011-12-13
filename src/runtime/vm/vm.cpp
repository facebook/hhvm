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
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/eval/runtime/file_repository.h>
#include <system/lib/systemlib.h>
#include <util/logger.h>

#include <libgen.h> // For dirname(3).
#include <string>
using namespace std;

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

class VMClassInfoHook : public ClassInfoHook {
public:
  virtual Array getUserFunctions() const {
    return g_context->getUserFunctionsInfo();
  }
  virtual Array getClasses() const {
    return g_context->getClassesInfo();
  }
  virtual Array getInterfaces() const {
    return g_context->getInterfacesInfo();
  }
  virtual Array getTraits() const {
    return g_context->getTraitsInfo();
  }
  virtual Array getConstants() const {
    return g_context->getConstantsInfo();
  }
  virtual const ClassInfo::MethodInfo *findFunction(CStrRef name) const {
    return g_context->findFunctionInfo(name);
  }
  virtual const ClassInfo *findClassLike(CStrRef name) const {
    const ClassInfo* ci;
    if ((ci = g_context->findClassInfo(name)) != NULL
        || (ci = g_context->findInterfaceInfo(name)) != NULL
        || (ci = g_context->findTraitInfo(name)) != NULL) {
      return ci;
    }
    return NULL;
  }
  virtual const ClassInfo *findInterface(CStrRef name) const {
    return g_context->findInterfaceInfo(name);
  }
  virtual const ClassInfo* findTrait(CStrRef name) const {
    return g_context->findTraitInfo(name);
  }
  virtual const ClassInfo::ConstantInfo *findConstant(CStrRef name) const {
    return g_context->findConstantInfo(name);
  }
};

static VMClassInfoHook vm_class_info_hook;

static HPHP::VM::Class* loadCppBuiltinClass(StringData* s,
                                            const HhbcExtClassInfo* info,
                                            const ClassInfo* ci) {
  StringData* parent = StringData::GetStaticString(ci->getParentClass().get());
  StringData* docComment = StringData::GetStaticString(ci->getDocComment());
  Location sLoc = Location();
  PreClass* preClass = SystemLib::s_unit->newPreClass(s, AttrPublic, parent,
                                                      docComment, &sLoc, 0,
                                                      true);
  {
    ClassInfo::InterfaceVec intfVec = ci->getInterfacesVec();
    for (unsigned i = 0; i < intfVec.size(); ++i) {
      const StringData* intf = StringData::GetStaticString(intfVec[i].get());
      preClass->addInterface(intf);
    }
  }
  for (long long i = 0; i < info->m_methodCount; i++) {
    const char* name = info->m_methods[i].m_name;
    const ClassInfo::MethodInfo* methInfo =
                                 ci->getMethodInfo(string(name));
    StringData* methName = StringData::GetStaticString(name);
    BuiltinClassFunction funcPtr = info->m_methods[i].m_pGenericMethod;
    Func* func = new Func(methName, methInfo, preClass, funcPtr);
    preClass->addMethod(func);
  }
  {
    ClassInfo::ConstantVec cnsVec = ci->getConstantsVec();
    for (unsigned i = 0; i < cnsVec.size(); ++i) {
      const ClassInfo::ConstantInfo* cnsInfo = cnsVec[i];
      ASSERT(cnsInfo);
      Variant val;
      try {
        val = cnsInfo->getValue();
      } catch (Exception& e) {
        ASSERT(false);
      }
      preClass->addConstant(cnsInfo->name.get(), (TypedValue*)(&val),
                            empty_string.get());
    }
  }
  Class* cls = g_context->defClass(preClass, false);
  ASSERT(cls);
  cls->addBuiltinClassInfo(info);
  return cls;
}

struct Entry {
  StringData* name;
  const HhbcExtClassInfo* info;
  const ClassInfo* ci;
};

void ProcessInit() {
  // Install VM's ClassInfoHook
  ClassInfo::SetHook(&vm_class_info_hook);
  FuncDict::ProcessInit();
  VM::Transl::Translator::Get()->processInit();

  // Search for systemlib.php in the following places:
  // 1) ${HPHP_LIB}/systemlib.php
  // 2) <dirname(realpath(hhvm))>/systemlib.php (requires proc filesystem)
  // 3) <HHVM_LIB_PATH_DEFAULT>/systemlib.php
  //
  // The rationale for this ordering is that ${HPHP_LIB} allows for a
  // run-time override, but under normal circumstances the other lookup methods
  // will be sufficient.  The realpath()-based lookup will succeed as long as
  // the proc filesystem exists (e.g. on Linux and some FreeBSD configurations)
  // and no hard links are in use for the executable.  Failing that, the
  // HHVM_LIB_PATH_DEFAULT-based lookup will always succeed, assuming that the
  // application was built and installed correctly.
  bool initial;
  String currentDir = g_context->getCwd();
  HPHP::Eval::PhpFile* file = NULL;

#define SYSTEMLIB_PHP "/systemlib.php"
#define LOOKUP_STR(s) do {                                                    \
  String systemlibPath = String(s) + SYSTEMLIB_PHP;                           \
  file = g_context->lookupPhpFile(systemlibPath.get(), currentDir.data(),     \
                                  initial);                                   \
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
  LOOKUP_ENV(HPHP_LIB);
  {
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

  // Load the systemlib unit to build the Class objects
  g_context->mergeUnit(SystemLib::s_unit);

#define INIT_SYSTEMLIB_CLASS_FIELD(cls) \
  { \
    ExecutionContext::DefinedClassMap::const_iterator it = \
      g_context->m_definedClasses.find(s_##cls.get()); \
    ASSERT(it != g_context->m_definedClasses.end()); \
    SystemLib::s_##cls##Class = it->second; \
  }

  // Stash a pointer to the VM Classes for stdclass, Exception,
  // pinitSentinel and resource
  INIT_SYSTEMLIB_CLASS_FIELD(stdclass);
  INIT_SYSTEMLIB_CLASS_FIELD(Exception);
  INIT_SYSTEMLIB_CLASS_FIELD(Directory);
  INIT_SYSTEMLIB_CLASS_FIELD(RecursiveDirectoryIterator);
  INIT_SYSTEMLIB_CLASS_FIELD(SplFileInfo);
  INIT_SYSTEMLIB_CLASS_FIELD(SplFileObject);
  INIT_SYSTEMLIB_CLASS_FIELD(pinitSentinel);
  INIT_SYSTEMLIB_CLASS_FIELD(resource);
  INIT_SYSTEMLIB_CLASS_FIELD(DOMException);
  INIT_SYSTEMLIB_CLASS_FIELD(PDOException);
  INIT_SYSTEMLIB_CLASS_FIELD(SoapFault);

#undef INIT_SYSTEMLIB_CLASS_FIELD

  // Build up extClassHash, a hashtable that maps class names to structures
  // containing C++ function pointers for the class's methods and constructors
  ASSERT(Class::s_extClassHash.size() == 0);
  for (long long i = 0LL; i < hhbc_ext_class_count; ++i) {
    const HhbcExtClassInfo* info = hhbc_ext_classes + i;
    StringData *s = StringData::GetStaticString(info->m_name);
    if (g_context->lookupClass(s) != NULL) {
      // Don't bother loading C++ classes when we already have pure ones
      continue;
    }
    Class::s_extClassHash[s] = info;
  }
  // If a given class has a base class, then we can't load that class
  // before we load the base class. Build up some structures so that
  // we can load the C++ builtin classes in the right order.
  vector<Entry> ready;
  typedef hphp_hash_map<StringData*, vector<Entry>,
                        string_data_hash, string_data_isame> PendingMap;
  PendingMap pending;
  hphp_hash_map<const StringData*, const HhbcExtClassInfo*,
                string_data_hash, string_data_isame>::iterator it;
  for (it = Class::s_extClassHash.begin();
       it != Class::s_extClassHash.end(); ++it) {
    Entry e;
    e.name = const_cast<StringData*>(it->first);
    e.info = it->second;
    e.ci = ClassInfo::FindClass(e.name);
    ASSERT(e.ci);
    StringData* parentName
      = StringData::GetStaticString(e.ci->getParentClass().get());
    if (parentName->empty()) {
      // If this class doesn't have a base class, it's ready to be
      // loaded now
      ready.push_back(e);
    } else {
      // If this class has a base class, we can't load it until its
      // base class has been loaded
      pending[parentName].push_back(e);
    }
  }
  for (unsigned k = 0; k < ready.size(); ++k) {
    Entry& e = ready[k];
    // Build a VM Class
    Class* cls = loadCppBuiltinClass(e.name, e.info, e.ci);
    // Stash a pointer to the Class in the appropriate static member
    // of SystemLib
    const ObjectStaticCallbacks* osc =
      get_object_static_callbacks(e.name->data());
    ASSERT(osc != NULL);
    *(osc->os_cls_ptr) = cls;
    // Any classes that derive from this class are now ready to be loaded
    PendingMap::iterator pendingIt = pending.find(e.name);
    if (pendingIt != pending.end()) {
      for (unsigned i = 0; i < pendingIt->second.size(); ++i) {
        ready.push_back(pendingIt->second[i]);
      }
      pending.erase(pendingIt);
    }
  }
  ASSERT(pending.size() == 0);
  SystemLib::s_inited = true;
}

///////////////////////////////////////////////////////////////////////////////
}
}
