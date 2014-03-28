/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/exception.h"

#ifdef HAVE_LIBDL
# include <dlfcn.h>
#include <map>
#include <vector>
# ifndef RTLD_LAZY
#  define RTLD_LAZY 1
# endif
# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif
# if defined(RTLD_GROUP) && defined(RTLD_WORLD) && defined(RTLD_PARENT)
#  define DLOPEN_FLAGS (RTLD_LAZY|RTLD_GLOBAL|RTLD_GROUP|RTLD_WORLD|RTLD_PARENT)
# else
#  define DLOPEN_FLAGS (RTLD_LAZY|RTLD_GLOBAL)
# endif
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Global systemlib extensions implemented entirely in PHP

IMPLEMENT_DEFAULT_EXTENSION_VERSION(redis, NO_EXTENSION_VERSION_YET);

///////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, Extension*, stdltistr> ExtensionMap;
static ExtensionMap *s_registered_extensions = NULL;
static bool s_modules_initialised = false;
static std::vector<Unit*> s_systemlib_units;

// just to make valgrind cleaner
class ExtensionUninitializer {
public:
  ~ExtensionUninitializer() {
    delete s_registered_extensions;
  }
};
static ExtensionUninitializer s_extension_uninitializer;

///////////////////////////////////////////////////////////////////////////////
// dlfcn wrappers

static void* dlopen(const char *dso) {
#ifdef HAVE_LIBDL
  return ::dlopen(dso, DLOPEN_FLAGS);
#else
  return nullptr;
#endif
}

static void* dlsym(void *mod, const char *sym) {
#ifdef HAVE_LIBDL
# ifdef LIBDL_NEEDS_UNDERSCORE
  std::string tmp("_");
  tmp += sym;
  sym = tmp.c_str();
# endif
  return ::dlsym(mod, sym);
#else
  return nullptr;
#endif
}

static const char* dlerror() {
#ifdef HAVE_LIBDL
  return ::dlerror();
#else
  return "Your system does not support dlopen()";
#endif
}

///////////////////////////////////////////////////////////////////////////////

Extension::Extension(litstr name, const char *version /* = "" */)
    : m_hhvmAPIVersion(HHVM_API_VERSION)
    , m_name(makeStaticString(name))
    , m_version(version ? version : "") {
  if (s_registered_extensions == NULL) {
    s_registered_extensions = new ExtensionMap();
  }
  assert(s_registered_extensions->find(name) ==
         s_registered_extensions->end());
  (*s_registered_extensions)[name] = this;
}

void Extension::LoadModules(Hdf hdf) {
  // Load up any dynamic extensions
  std::string path = hdf["DynamicExtensionPath"].getString(".");
  for (Hdf ext = hdf["DynamicExtensions"].firstChild();
       ext.exists(); ext = ext.next()) {
    std::string extLoc = ext.getString();
    if (extLoc.empty()) {
      continue;
    }
    if (extLoc[0] != '/') {
      extLoc = path + "/" + extLoc;
    }

    // Extensions are self-registering,
    // so we bring in the SO then
    // throw away its handle.
    void *ptr = dlopen(extLoc.c_str());
    if (!ptr) {
      throw Exception("Could not open extension %s: %s",
                      extLoc.c_str(), dlerror());
    }
    auto getModule = (Extension *(*)())dlsym(ptr, "getModule");
    if (!getModule) {
      throw Exception("Could not load extension %s: %s (%s)",
                      extLoc.c_str(),
                      "getModule() symbol not defined.",
                      dlerror());
    }
    Extension *mod = getModule();
    if (mod->m_hhvmAPIVersion != HHVM_API_VERSION) {
      throw Exception("Could not use extension %s: "
                      "Compiled with HHVM API Version %" PRId64 ", "
                      "this version of HHVM expects %ld",
                      extLoc.c_str(),
                      mod->m_hhvmAPIVersion,
                      HHVM_API_VERSION);
    }
    mod->setDSOName(extLoc);
  }

  // Invoke Extension::moduleLoad() callbacks
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->moduleLoad(hdf);
  }
}

void Extension::InitModules() {
  assert(s_registered_extensions);
  bool wasInited = SystemLib::s_inited;
  LitstrTable::get().setWriting();
  auto const wasDB = RuntimeOption::EvalDumpBytecode;
  RuntimeOption::EvalDumpBytecode &= ~1;
  SCOPE_EXIT {
    SystemLib::s_inited = wasInited;
    LitstrTable::get().setReading();
    RuntimeOption::EvalDumpBytecode = wasDB;
  };
  SystemLib::s_inited = false;
  for (auto& kv : *s_registered_extensions) {
    kv.second->moduleInit();
  }
  s_modules_initialised = true;
}

void Extension::ThreadInitModules() {
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->threadInit();
  }
}

void Extension::ThreadShutdownModules() {
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->threadShutdown();
  }
}

void Extension::RequestInitModules() {
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->requestInit();
  }
}

void Extension::RequestShutdownModules() {
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->requestShutdown();
  }
}

bool Extension::ModulesInitialised() {
  return s_modules_initialised;
}

void Extension::ShutdownModules() {
  assert(s_registered_extensions);
  for (auto& kv : *s_registered_extensions) {
    kv.second->moduleShutdown();
  }
  s_registered_extensions->clear();
}

const StaticString s_apc("apc");

bool Extension::IsLoaded(const String& name) {
  if (name == s_apc) {
    return apcExtension::Enable;
  }
  assert(s_registered_extensions);
  return s_registered_extensions->find(name.data()) !=
    s_registered_extensions->end();
}

Extension *Extension::GetExtension(const String& name) {
  assert(s_registered_extensions);
  ExtensionMap::iterator iter = s_registered_extensions->find(name.data());
  if (iter != s_registered_extensions->end()) {
    return iter->second;
  }
  return NULL;
}

Array Extension::GetLoadedExtensions() {
  assert(s_registered_extensions);
  Array ret = Array::Create();
  for (auto& kv : *s_registered_extensions) {
    if (!apcExtension::Enable && kv.second->m_name == s_apc) {
      continue;
    }
    ret.append(kv.second->m_name);
  }
  return ret;
}

void Extension::MergeSystemlib() {
  for (auto &unit : s_systemlib_units) {
    unit->merge();
  }
}

void Extension::CompileSystemlib(const std::string &slib,
                                 const std::string &name) {
  // TODO (t3443556) Bytecode repo compilation expects that any errors
  // encountered during systemlib compilation have valid filename pointers
  // which won't be the case for now unless these pointers are long-lived.
  auto const moduleName = makeStaticString(name.c_str());
  Unit *unit = compile_systemlib_string(slib.c_str(), slib.size(),
                                        moduleName->data());
  assert(unit);
  unit->merge();
  s_systemlib_units.push_back(unit);
}

/**
 * Loads a named systemlib section from the main binary (or DSO)
 * using the label "ext.{hash(name)}"
 *
 * If {name} is not passed, then {m_name} is assumed.
 */
void Extension::loadSystemlib(const std::string& name /*= "" */) {
  std::string n = name.empty() ?
    std::string(m_name.data(), m_name.size()) : name;
  std::string section("ext.");
  section += f_md5(n, false).substr(0, 12).data();
  std::string hhas, slib = get_systemlib(&hhas, section, m_dsoName);
  if (!slib.empty()) {
    std::string phpname("systemlib.php");
    phpname += n;
    CompileSystemlib(slib, phpname);
  }
  if (!hhas.empty()) {
    std::string hhasname("systemlib.hhas.");
    hhasname += n;
    CompileSystemlib(hhas, hhasname);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Extension argument API

static void countArgs(const char *format, unsigned &min, unsigned &max) {
  bool required = true;
  min = max = 0;
  while (auto c = *(format++)) {
    if (c == '|') {
      required = false;
      continue;
    }
    if (c == '!') {
      continue;
    }
    if (required) min++;
    max++;
  }
}

static const char *argTypeName(DataType dt) {
  switch (dt) {
    case KindOfNull: return "null";
    case KindOfBoolean: return "boolean";
    case KindOfInt64: return "integer";
    case KindOfDouble: return "double";
    case KindOfString:
    case KindOfStaticString: return "string";
    case KindOfArray: return "array";
    case KindOfObject: return "object";
    case KindOfResource: return "resource";
    default: return "unknown";
  }
  not_reached();
}

template <DataType DType, class T>
void parseArgValue(TypedValue *tv,
                   va_list va, bool check_null) {
  T* pval = va_arg(va, T*);
  if (check_null) {
     *va_arg(va, bool*) = (tv->m_type == KindOfNull);
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    throw_invalid_argument("Expected %s, got %s",
                           argTypeName(DType),
                           argTypeName(tv->m_type));
    tvCastInPlace(tv, DType);
  }
  *pval = unpack_tv<DType>(tv);
}

template <DataType DType, class T>
bool parseArgPointer(TypedValue *tv,
                     va_list va, bool check_null) {
  T* pval = va_arg(va, T*);
  if (check_null && (tv->m_type == KindOfNull)) {
    *pval = nullptr;
    return true;
  }
  if (tv->m_type != DType) {
    throw_invalid_argument("Expected %s, got %s",
                           argTypeName(DType),
                           argTypeName(tv->m_type));
    return false;
  }
  *pval = unpack_tv<DType>(tv);
  return true;
}

#define PARSE_ARG_VAL(fmt, dt) \
  case fmt: \
    parseArgValue<dt, typename DataTypeCPPType<dt>::type> \
      (tv, va, check_null); break;

#define PARSE_ARG_PTR(fmt, dt) \
  case fmt: \
    if (!parseArgPointer<dt, typename DataTypeCPPType<dt>::type> \
      (tv, va, check_null)) { return false; } break;

bool parseArgs(ActRec *ar, const char *format, ...) {
  unsigned min, max, count = ar->numArgs();
  countArgs(format, min, max);
  if (count < min) {
    throw_wrong_arguments_nr(ar->func()->name()->data(), count, min, max);
    return false;
  }

  unsigned arg = 0;
  va_list va;
  va_start(va, format);
  SCOPE_EXIT { va_end(va); };

  while (auto c = *(format++)) {
    if (c == '|' || c == '!') {
      continue;
    }

    if (arg >= count) {
      // Still have format specs, but no more args passed
      // throw_wrong_arguments_nr check should guarantee
      // that we're already past min args
      assert(arg >= min);
      break;
    }

    bool check_null = (format[0] == '!');
    TypedValue *tv = getArg(ar, arg++);

    switch (c) {
      PARSE_ARG_VAL('b', KindOfBoolean);
      PARSE_ARG_VAL('l', KindOfInt64);
      PARSE_ARG_VAL('d', KindOfDouble);
      PARSE_ARG_PTR('r', KindOfResource);
      PARSE_ARG_PTR('a', KindOfArray);
      PARSE_ARG_PTR('o', KindOfObject);

      case 's': { // KindOfString
        StringData **psval = va_arg(va, StringData**);
        if (check_null && (tv->m_type == KindOfNull)) {
          *psval = nullptr;
          break;
        }
        if (!tvCoerceParamInPlace(tv, KindOfString)) {
          throw_invalid_argument("Expected string, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        *psval = unpack_tv<KindOfString>(tv);
        break;
      }

      case 'O': { // KindOfObject (specific type)
        ObjectData **objval = va_arg(va, ObjectData**);
        Class *expClass = va_arg(va, Class*);
        if (check_null && (tv->m_type == KindOfNull)) {
          *objval = nullptr;
          break;
        }
        if (tv->m_type != KindOfObject) {
          throw_invalid_argument("Expected %s, got %s",
                                 expClass->name()->data(),
                                 argTypeName(tv->m_type));
          return false;
        }
        auto odata = unpack_tv<KindOfObject>(tv);
        Class *cls = odata->getVMClass();
        if ((cls != expClass) && !cls->classof(expClass)) {
          throw_invalid_argument("Expected %s, got %s",
                                 expClass->name()->data(),
                                 cls->name()->data());
          return false;
        }
        *objval = odata;
        break;
      }

      case 'C': { // KindOfClass
        Class **clsval = va_arg(va, Class**);
        if (check_null && (tv->m_type == KindOfNull)) {
          *clsval = nullptr;
          break;
        }
        if (!tvCoerceParamInPlace(tv, KindOfString)) {
          throw_invalid_argument("Expected string class name, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        auto cls = Unit::loadClass(tv->m_data.pstr);
        if (!cls) {
          throw_invalid_argument("Unknown class %s",
                                 tv->m_data.pstr->data());
          return false;
        }
        *clsval = cls;
        break;
      }

      case 'A': // KindOfArray || KindOfObject
        if ((tv->m_type != KindOfArray) &&
            (tv->m_type != KindOfObject)) {
          throw_invalid_argument("Expected array or object, got %s",
                                 argTypeName(tv->m_type));
          return false;
        }
        /* fallthrough */
      case 'v': // KindOfAny (Variant)
        *va_arg(va, Variant*) = tv ? tvAsVariant(tv) : uninit_null();
        break;

      case 'V': // KindOfAny (TypedValue*)
        *va_arg(va, TypedValue**) = tv;

      default:
        not_reached();
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
