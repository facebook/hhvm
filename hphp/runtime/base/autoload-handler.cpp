/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/autoload-handler.h"

#include <algorithm>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_class("class"),
  s_function("function"),
  s_constant("constant"),
  s_type("type"),
  s_failure("failure"),
  s_autoload("__autoload"),
  s_exception("exception"),
  s_previous("previous");

using CufIterPtr = req::unique_ptr<CufIter>;

//////////////////////////////////////////////////////////////////////

Variant invoke_for_autoload(const String& function, const Variant& params) {
  Func* func = Unit::loadFunc(function.get());
  if (func && (isContainer(params) || params.isNull())) {
    Variant ret;
    g_context->invokeFunc(ret.asTypedValue(), func, params);
    return ret;
  }
  raise_warning("call_user_func to non-existent function %s",
    function.c_str());
  return Variant(false);
}

/*
 * Wraps calling an (autoload) PHP function from a CufIter.
 */
Variant vm_call_user_func_cufiter(const CufIter& cufIter,
                                  const Array& params) {
  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = cufIter.name();
  const HPHP::Func* f = cufIter.func();
  if (cufIter.ctx()) {
    if (uintptr_t(cufIter.ctx()) & 1) {
      cls = (Class*)(uintptr_t(cufIter.ctx()) & ~1);
    } else {
      obj = (ObjectData*)cufIter.ctx();
    }
  }
  assert(!obj || !cls);
  if (invName) {
    invName->incRefCount();
  }
  Variant ret;
  g_context->invokeFunc((TypedValue*)&ret, f, params, obj, cls,
                          nullptr, invName, ExecutionContext::InvokeCuf);
  return ret;
}

/*
 * Helper method from converting between a PHP function and a CufIter.
 */
bool vm_decode_function_cufiter(const Variant& function,
                                CufIterPtr& cufIter) {
  ObjectData* obj = nullptr;
  Class* cls = nullptr;
  CallerFrame cf;
  StringData* invName = nullptr;
  // Don't warn here, let the caller decide what to do if the func is nullptr.
  const HPHP::Func* func = vm_decode_function(function, cf(), false,
                                              obj, cls, invName, false);
  if (func == nullptr) {
    return false;
  }

  cufIter = req::make_unique<CufIter>();
  cufIter->setFunc(func);
  cufIter->setName(invName);
  if (obj) {
    cufIter->setCtx(obj);
    obj->incRefCount();
  } else {
    cufIter->setCtx(cls);
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

void AutoloadHandler::requestInit() {
  assert(m_map.get() == nullptr);
  assert(m_map_root.get() == nullptr);
  assert(m_loading.get() == nullptr);
  m_spl_stack_inited = false;
  new (&m_handlers) req::deque<HandlerBundle>();
}

void AutoloadHandler::requestShutdown() {
  m_map.reset();
  m_map_root.reset();
  m_loading.reset();
  // m_spl_stack_inited will be re-initialized by the next requestInit
  // m_handlers will be re-initialized by the next requestInit
}

bool AutoloadHandler::setMap(const Array& map, const String& root) {
  this->m_map = map;
  this->m_map_root = root;
  return true;
}

namespace {
struct FuncExistsChecker {
  const StringData* m_name;
  mutable NamedEntity* m_ne;
  explicit FuncExistsChecker(const StringData* name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name, false);
      if (!m_ne) {
        return false;
      }
    }
    auto f = m_ne->getCachedFunc();
    return (f != nullptr) &&
           (f->builtinFuncPtr() != Native::unimplementedWrapper);
  }
};
struct ClassExistsChecker {
  const String& m_name;
  mutable NamedEntity* m_ne;
  explicit ClassExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name.get(), false);
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedClass() != nullptr;
  }
};
struct ConstExistsChecker {
  const StringData* m_name;
  explicit ConstExistsChecker(const StringData* name)
    : m_name(name) {}
  bool operator()() const {
    return Unit::lookupCns(m_name) != nullptr;
  }
};
struct TypeExistsChecker {
  const String& m_name;
  mutable NamedEntity* m_ne;
  explicit TypeExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name.get(), false);
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedTypeAlias() != nullptr;
  }
};
struct ClassOrTypeExistsChecker {
  const String& m_name;
  mutable NamedEntity* m_ne;
  explicit ClassOrTypeExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name.get(), false);
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedClass() != nullptr ||
           m_ne->getCachedTypeAlias() != nullptr;
  }
};
}

const StaticString
  s_file("file"),
  s_line("line");

template <class T>
AutoloadHandler::Result
AutoloadHandler::loadFromMapImpl(const String& clsName,
                                 const String& kind,
                                 bool toLower,
                                 const T &checkExists,
                                 Variant& err) {
  assert(!m_map.isNull());
  // Always normalize name before autoloading
  const String& name = normalizeNS(clsName);
  const Variant& type_map = m_map.get()->get(kind);
  auto const typeMapCell = type_map.asCell();
  if (typeMapCell->m_type != KindOfArray) return Failure;
  String canonicalName = toLower ? HHVM_FN(strtolower)(name) : name;
  const Variant& file = typeMapCell->m_data.parr->get(canonicalName);
  bool ok = false;
  if (file.isString()) {
    String fName = file.toCStrRef().get();
    if (fName.get()->data()[0] != '/') {
      if (!m_map_root.empty()) {
        fName = m_map_root + fName;
      }
    }
    try {
      VMRegAnchor _;
      bool initial;
      auto const ec = g_context.getNoCheck();
      auto const unit = lookupUnit(fName.get(), "", &initial);
      if (unit) {
        if (initial) {
          TypedValue retval;
          ec->invokeFunc(&retval, unit->getMain(), init_null_variant,
                         nullptr, nullptr, nullptr, nullptr,
                         ExecutionContext::InvokePseudoMain);
          tvRefcountedDecRef(&retval);
        }
        ok = true;
      }
    } catch (ExitException&) {
      throw;
    } catch (ResourceExceededException&) {
      throw;
    } catch (ExtendedException& ee) {
      auto fileAndLine = ee.getFileAndLine();
      err = (fileAndLine.first.empty())
        ? ee.getMessage()
        : folly::format("{} in {} on line {}",
                        ee.getMessage(), fileAndLine.first,
                        fileAndLine.second).str();
    } catch (Exception& e) {
      err = e.getMessage();
    } catch (Object& e) {
      err = e;
    } catch (...) {
      err = String("Unknown Exception");
    }
  }
  if (ok && checkExists()) {
    return Success;
  }
  return Failure;
}

template <class T>
AutoloadHandler::Result
AutoloadHandler::loadFromMap(const String& clsName,
                             const String& kind,
                             bool toLower,
                             const T &checkExists) {
  while (true) {
    Variant err{Variant::NullInit()};
    Result res = loadFromMapImpl(clsName, kind, toLower, checkExists, err);
    if (res == Success) return Success;
    const Variant& func = m_map.get()->get(s_failure);
    if (func.isNull()) return Failure;
    res = invokeFailureCallback(func, kind, clsName, err);
    if (checkExists()) return Success;
    if (res == RetryAutoloading) {
      continue;
    }
    return res;
  }
}

AutoloadHandler::Result
AutoloadHandler::invokeFailureCallback(const Variant& func, const String& kind,
                                       const String& name, const Variant& err) {
  // can throw, otherwise
  //  - true means the map was updated. try again
  //  - false means we should stop applying autoloaders (only affects classes)
  //  - anything else means keep going
  Variant action = vm_call_user_func(func,
                                     make_packed_array(kind, name, err));
  auto const actionCell = action.asCell();
  if (actionCell->m_type == KindOfBoolean) {
    return actionCell->m_data.num ? RetryAutoloading : StopAutoloading;
  }
  return ContinueAutoloading;
}

bool AutoloadHandler::autoloadFunc(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_function, true, FuncExistsChecker(name)) != Failure;
}

bool AutoloadHandler::autoloadConstant(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_constant, false, ConstExistsChecker(name)) != Failure;
}

bool AutoloadHandler::autoloadType(const String& name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_type, true, TypeExistsChecker(name)) != Failure;
}

/**
 * Taken from php-src
 * https://github.com/php/php-src/blob/PHP-5.6/Zend/zend_execute_API.c#L960
 */
static bool is_valid_class_name(const String& className) {
  return strspn(
    className.data(),
    "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\177"
    "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220"
    "\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241"
    "\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262"
    "\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303"
    "\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324"
    "\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345"
    "\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366"
    "\367\370\371\372\373\374\375\376\377\\"
  ) == className.length();
}

bool AutoloadHandler::autoloadClass(const String& clsName,
                                    bool forceSplStack /* = false */) {
  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);
  // Verify class name before passing it to __autoload()
  if (!is_valid_class_name(className)) {
    return false;
  }
  if (!m_map.isNull()) {
    ClassExistsChecker ce(className);
    Result res = loadFromMap(className, s_class, true, ce);
    if (res == Success || ce()) return true;
    if (res == StopAutoloading) return false;
  }
  return autoloadClassPHP5Impl(className, forceSplStack);
}

bool AutoloadHandler::autoloadClassPHP5Impl(const String& className,
                                            bool forceSplStack) {
  // If we end up in a recursive autoload loop where we try to load the
  // same class twice, just fail the load to match PHP5 as many frameworks
  // rely on it unless we are forcing a restart (due to spl_autoload_call)
  // in which case autoload is allowed to be reentrant.
  if (!forceSplStack) {
    if (m_loading.exists(className)) { return false; }
    m_loading.add(className, className);
  } else {
    // We can still overflow the stack if there is a loop when using
    // spl_autoload_call directly, but this behavior matches PHP5.
    m_loading.append(className);
  }

  // Make sure state is cleaned up from this load; autoloading of arbitrary
  // code below can throw
  SCOPE_EXIT {
    String l_className = m_loading.pop();
    assert(l_className == className);
  };

  Array params = PackedArrayInit(1).append(className).toArray();
  if (!m_spl_stack_inited && !forceSplStack) {
    if (function_exists(s_autoload)) {
      invoke_for_autoload(s_autoload, params);
      return true;
    }
    return false;
  }
  if (!m_spl_stack_inited || m_handlers.empty()) {
    return false;
  }
  Object autoloadException;
  for (const HandlerBundle& hb : m_handlers) {
    try {
      vm_call_user_func_cufiter(*hb.m_cufIter, params);
    } catch (Object& ex) {
      assert(ex.instanceof(SystemLib::s_ExceptionClass));
      if (autoloadException.isNull()) {
        autoloadException = ex;
      } else {
        Object cur = ex;
        Variant next = cur->o_get(s_previous, false, s_exception);
        while (next.isObject()) {
          cur = next.toObject();
          next = cur->o_get(s_previous, false, s_exception);
        }
        cur->o_set(s_previous, autoloadException, s_exception);
        autoloadException = ex;
      }
    }
    if (Unit::lookupClass(className.get()) != nullptr) {
      break;
    }
  }
  if (!autoloadException.isNull()) {
    throw autoloadException;
  }
  return true;
}

template <class T>
AutoloadHandler::Result
AutoloadHandler::loadFromMapPartial(const String& className,
                                    const String& kind,
                                    bool toLower,
                                    const T &checkExists,
                                    Variant& err) {
  Result res = loadFromMapImpl(className, kind, toLower, checkExists, err);
  if (res == Success) {
    return Success;
  }
  assert(res == Failure);
  if (!err.isNull()) {
    const Variant& func = m_map.get()->get(s_failure);
    if (!func.isNull()) {
      res = invokeFailureCallback(func, kind, className, err);
      assert(res != Failure);
      if (checkExists()) {
        return Success;
      }
    }
  }
  return res;
}

bool AutoloadHandler::autoloadClassOrType(const String& clsName) {
  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);
  if (!m_map.isNull()) {
    ClassOrTypeExistsChecker cte(className);
    bool tryClass = true, tryType = true;
    Result classRes = RetryAutoloading, typeRes = RetryAutoloading;
    while (true) {
      Variant classErr{Variant::NullInit()};
      if (tryClass) {
        // Try consulting the 'class' map first, but don't call the failure
        // callback unless there was an uncaught exception or a fatal error
        // during the include operation.
        classRes = loadFromMapPartial(className, s_class, true, cte, classErr);
        if (classRes == Success) return true;
      }
      Variant typeErr{Variant::NullInit()};
      if (tryType) {
        // Next, try consulting the 'type' map. Again, don't call the failure
        // callback unless there was an uncaught exception or fatal error.
        typeRes = loadFromMapPartial(className, s_type, true, cte, typeErr);
        if (typeRes == Success) return true;
      }
      const Variant& func = m_map.get()->get(s_failure);
      // If we reach this point, then for each map either nothing was found
      // or the file we included didn't define a class or type alias with the
      // specified name, and the failure callback (if one exists) did not throw
      // or raise a fatal error.
      if (!func.isNull()) {
        // First, call the failure callback for 'class' if we didn't do so
        // above
        if (classRes == Failure) {
          assert(tryClass);
          classRes = invokeFailureCallback(func, s_class, className, classErr);
          // The failure callback may have defined a class or type alias for
          // us, in which case we're done.
          if (cte()) return true;
        }
        // Next, call the failure callback for 'type' if we didn't do so above
        if (typeRes == Failure) {
          assert(tryType);
          typeRes = invokeFailureCallback(func, s_type, className, typeErr);
          // The failure callback may have defined a class or type alias for
          // us, in which case we're done.
          if (cte()) return true;
        }
        assert(classRes != Failure && typeRes != Failure);
        tryClass = (classRes == RetryAutoloading);
        tryType = (typeRes == RetryAutoloading);
        // If the failure callback requested a retry for 'class' or 'type'
        // or both, jump back to the top to try again.
        if (tryClass || tryType) {
          continue;
        }
        if (classRes == StopAutoloading) {
          // If the failure callback requested that we stop autoloading for
          // 'class', then return false here so we don't fall through to the
          // PHP5 autoload impl below.
          return false;
        }
      }
      // Break out of the while loop so that we can fall through to the
      // to the call the the PHP5 autoload impl below.
      break;
    }
  }
  return autoloadClassPHP5Impl(className, false);
}

Array AutoloadHandler::getHandlers() {
  if (!m_spl_stack_inited) {
    return Array();
  }

  PackedArrayInit handlers(m_handlers.size());

  for (const HandlerBundle& hb : m_handlers) {
    CufIter* cufIter = hb.m_cufIter.get();
    ObjectData* obj = nullptr;
    HPHP::Class* cls = nullptr;
    const HPHP::Func* f = cufIter->func();

    if (hb.m_handler.isObject()) {
      handlers.append(hb.m_handler);
    } else if (cufIter->ctx()) {
      PackedArrayInit callable(2);
      if (uintptr_t(cufIter->ctx()) & 1) {
        cls = (Class*)(uintptr_t(cufIter->ctx()) & ~1);
        callable.append(String(cls->nameStr()));
      } else {
        obj = (ObjectData*)cufIter->ctx();
        callable.append(obj);
      }
      callable.append(String(f->nameStr()));
      handlers.append(callable.toArray());
    } else {
      handlers.append(String(f->nameStr()));
    }
  }

  return handlers.toArray();
}

bool AutoloadHandler::CompareBundles::operator()(
  const HandlerBundle& hb) {
  auto const& lhs = *m_cufIter;
  auto const& rhs = *hb.m_cufIter;

  if (lhs.ctx() != rhs.ctx()) {
    // We only consider ObjectData* for equality (not a Class*) so if either is
    // an object these are not considered equal.
    if (!(uintptr_t(lhs.ctx()) & 1) || !(uintptr_t(rhs.ctx()) & 1)) {
      return false;
    }
  }

  return lhs.func() == rhs.func();
}

bool AutoloadHandler::addHandler(const Variant& handler, bool prepend) {
  CufIterPtr cufIter = nullptr;
  if (!vm_decode_function_cufiter(handler, cufIter)) {
    return false;
  }

  m_spl_stack_inited = true;

  // Zend doesn't modify the order of the list if the handler is already
  // registered.
  auto const& compareBundles = CompareBundles(cufIter.get());
  if (std::find_if(m_handlers.begin(), m_handlers.end(), compareBundles) !=
      m_handlers.end()) {
    return true;
  }

  if (!prepend) {
    m_handlers.emplace_back(handler, cufIter);
  } else {
    m_handlers.emplace_front(handler, cufIter);
  }

  return true;
}

bool AutoloadHandler::isRunning() {
  return !m_loading.empty();
}

void AutoloadHandler::removeHandler(const Variant& handler) {
  CufIterPtr cufIter = nullptr;
  if (!vm_decode_function_cufiter(handler, cufIter)) {
    return;
  }

  // Use find_if instead of remove_if since we know there can only be one match
  // in the vector.
  auto const& compareBundles = CompareBundles(cufIter.get());
  auto it = std::find_if(m_handlers.begin(), m_handlers.end(), compareBundles);
  if (it != m_handlers.end()) {
    m_handlers.erase(it);
  }
}

void AutoloadHandler::removeAllHandlers() {
  m_spl_stack_inited = false;
  m_handlers.clear();
}


//////////////////////////////////////////////////////////////////////

}
