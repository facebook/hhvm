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

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/ext/ext_string.h"
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

using SmartCufIterPtr = smart::unique_ptr<CufIter>;

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
                                SmartCufIterPtr& cufIter) {
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

  cufIter = smart::make_unique<CufIter>();
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
  new (&m_handlers) smart::deque<HandlerBundle>();
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
struct ClassExistsChecker {
  bool operator()(const String& name) const {
    return Unit::lookupClass(name.get()) != nullptr;
  }
};
struct ConstantExistsChecker {
  bool operator()(const String& name) const {
    return Unit::lookupCns(name.get()) != nullptr;
  }
};
}

const StaticString
  s_file("file"),
  s_line("line");

template <class T>
AutoloadHandler::Result AutoloadHandler::loadFromMap(const String& clsName,
                                                     const String& kind,
                                                     bool toLower,
                                                     const T &checkExists) {
  assert(!m_map.isNull());

  // always normalize name before autoloading
  const String& name = normalizeNS(clsName);

  while (true) {
    const Variant& type_map = m_map.get()->get(kind);
    auto const typeMapCell = type_map.asCell();
    if (typeMapCell->m_type != KindOfArray) return Failure;
    String canonicalName = toLower ? f_strtolower(name) : name;
    const Variant& file = typeMapCell->m_data.parr->get(canonicalName);
    bool ok = false;
    Variant err{Variant::NullInit()};
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
      } catch (ExtendedException& ee) {
        auto fileAndLine = ee.getFileAndLine();
        err = (fileAndLine.first.empty()) ? ee.getMessage()
          : folly::format("{0} in {1} on line {2}",
                          ee.getMessage(), fileAndLine.first.c_str(),
                          fileAndLine.second).str();
      } catch (Exception& e) {
        err = e.getMessage();
      } catch (Object& e) {
        err = e;
      } catch (...) {
        err = String("Unknown Exception");
      }
    }
    if (ok && checkExists(name)) {
      return Success;
    }
    const Variant& func = m_map.get()->get(s_failure);
    if (func.isNull()) return Failure;
    // can throw, otherwise
    //  - true means the map was updated. try again
    //  - false means we should stop applying autoloaders (only affects classes)
    //  - anything else means keep going
    Variant action = vm_call_user_func(func,
                                       make_packed_array(kind, name, err));
    auto const actionCell = action.asCell();
    if (actionCell->m_type == KindOfBoolean) {
      if (actionCell->m_data.num) continue;
      return StopAutoloading;
    }
    return ContinueAutoloading;
  }
}

bool AutoloadHandler::autoloadFunc(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_function, true, function_exists) != Failure;
}

bool AutoloadHandler::autoloadConstant(StringData* name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_constant, false, ConstantExistsChecker()) != Failure;
}

bool AutoloadHandler::autoloadType(const String& name) {
  return !m_map.isNull() &&
    loadFromMap(name, s_type, true,
      [] (const String& name) {
        return Unit::GetNamedEntity(name.get())->
          getCachedTypeAlias() != nullptr;
      }
    ) != Failure;
}

/**
 * invokeHandler returns true if any autoload handlers were executed,
 * false otherwise. When this function returns true, it is the caller's
 * responsibility to check if the given class or interface exists.
 */
bool AutoloadHandler::invokeHandler(const String& clsName,
                                    bool forceSplStack /* = false */) {

  if (clsName.empty()) return false;

  const String& className = normalizeNS(clsName);

  if (!m_map.isNull()) {
    ClassExistsChecker ce;
    Result res = loadFromMap(className, s_class, true, ce);
    if (res == ContinueAutoloading) {
      if (ce(className)) return true;
    } else {
      if (res != Failure) return res == Success;
    }
  }
  // If we end up in a recursive autoload loop where we try to load the
  // same class twice, just fail the load to mimic PHP as many frameworks
  // rely on it unless we are forcing a restart (due to spl_autoload_call)
  // in which case autoload is allowed to be reentrant.
  if (!forceSplStack) {
    if (m_loading.exists(className)) { return false; }
    m_loading.add(className, className);
  } else {
    // We can still overflow the stack if there is a loop when using
    // spl_autoload_call directly, but this behavior matches the reference
    // implementation.
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

Array AutoloadHandler::getHandlers() {
  if (!m_spl_stack_inited) {
    return Array();
  }

  PackedArrayInit handlers(m_handlers.size());

  for (const HandlerBundle& hb : m_handlers) {
    handlers.append(hb.m_handler);
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
  SmartCufIterPtr cufIter = nullptr;
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
  SmartCufIterPtr cufIter = nullptr;
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
