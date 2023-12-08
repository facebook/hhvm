/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/recorder.h"
#include "hphp/runtime/base/replayer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/repo-autoload-map.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

RDS_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

static FactsFactory* s_mapFactory = nullptr;

std::unique_ptr<RepoAutoloadMap> AutoloadHandler::s_repoAutoloadMap{};

FactsFactory* FactsFactory::getInstance() {
  return s_mapFactory;
}

void FactsFactory::setInstance(FactsFactory* instance) {
  s_mapFactory = instance;
}

//////////////////////////////////////////////////////////////////////

namespace {

FactsStore* getFactsForRequest() {
  if (UNLIKELY(RO::EvalRecordReplay && RO::EvalReplay)) {
    return Replayer::onGetFactsForRequest();
  }

  auto* factory = FactsFactory::getInstance();
  if (!factory) {
    return nullptr;
  }

  if (g_context.isNull()) {
    return nullptr;
  }

  auto* repoOptions = g_context->getRepoOptionsForRequest();
  if (!repoOptions || repoOptions->path().empty()) {
    return nullptr;
  }

  auto* map = factory->getForOptions(*repoOptions);
  if (!map) {
    return nullptr;
  }

  try {
    tracing::Block _{"autoload-ensure-updated"};
    map->ensureUpdated();
    if (UNLIKELY(RO::EvalRecordReplay && RO::EvalRecordSampleRate)) {
      Recorder::onGetFactsForRequest(map);
    }
    return map;
  } catch (const std::exception& e) {
    auto repoRoot = repoOptions->dir();
    Logger::FError(
        "Failed to update native autoloader, not natively autoloading {}. {}\n",
        repoRoot.native(),
        e.what());
  }
  return nullptr;
}

} // namespace

void AutoloadHandler::requestInit() {
  assertx(!m_map);
  assertx(!m_facts);
  assertx(m_onPostAutoloadFunc.isNull());
  m_facts = getFactsForRequest();
  if (RuntimeOption::RepoAuthoritative) {
    m_map = s_repoAutoloadMap.get();
    assertx(m_map);
  } else {
    m_map = m_facts;
  }
}

void AutoloadHandler::requestShutdown() {
  m_map = nullptr;
  m_facts = nullptr;
  m_onPostAutoloadFunc.setNull();
}

namespace {
struct FuncExistsChecker {
  const StringData* m_name;
  mutable NamedFunc* m_ne;
  explicit FuncExistsChecker(const StringData* name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedFunc::getNoCreate(m_name);
      if (!m_ne) {
        return false;
      }
    }
    auto f = m_ne->getCachedFunc();
    return f != nullptr;
  }
};

struct ClassExistsChecker {
  const String& m_name;
  mutable NamedType* m_ne;
  explicit ClassExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedType::getNoCreate(m_name.get());
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
    return type(Constant::lookup(m_name)) != KindOfUninit;
  }
};

struct TypeAliasExistsChecker {
  const String& m_name;
  mutable NamedType* m_ne;
  explicit TypeAliasExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedType::getNoCreate(m_name.get());
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedTypeAlias() != nullptr;
  }
};

struct NamedTypeExistsChecker {
  const String& m_name;
  mutable NamedType* m_ne;
  explicit NamedTypeExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedType::getNoCreate(m_name.get());
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedClass() != nullptr ||
           m_ne->getCachedTypeAlias() != nullptr;
  }
};

struct ModuleExistsChecker {
  const StringData* m_name;
  explicit ModuleExistsChecker(const StringData* name)
    : m_name(name) {}
  bool operator()() const {
    return Module::lookup(m_name) != nullptr;
  }
};

} // namespace

const StaticString
  s_file("file"),
  s_line("line");

Optional<AutoloadMap::FileResult> AutoloadHandler::getFile(
  const String& clsName, AutoloadMap::KindOf kind) {
  assertx(m_map);
  // Always normalize name before autoloading
  return m_map->getFile(kind, normalizeNS(clsName));
}

template <class T>
bool
AutoloadHandler::loadFromMapImpl(const String& clsName,
                                 AutoloadMap::KindOf kind,
                                 const T &checkExists,
                                 Variant& err) {
  auto fileRes = getFile(clsName, kind);
  if (!fileRes) {
    return false;
  }
  bool ok = false;
  // Utility for logging errors in server mode.
  auto log_err = [](char const* const msg) {
    if (RuntimeOption::ServerMode) {
      Logger::Error("Exception: AutoloadMap::loadFromMapImpl: %s", msg);
    }
  };
  try {
    VMRegAnchor _;
    bool initial;
    auto const eagerSync = RO::EvalAutoloadEagerSyncUnitCache && m_map;
    auto const unit = lookupUnit(fileRes->path.get(), fileRes->info, "",
                                 &initial, nullptr,
                                 RuntimeOption::TrustAutoloaderPath,
                                 false /* forPrefetch */,
                                 eagerSync /* forAutoload */);
    if (unit) {
      if (initial) unit->merge();
      ok = true;
    }
  } catch (ExitException& ee) {
    throw;
  } catch (ResourceExceededException& ree) {
    throw;
  } catch (ExtendedException& ee) {
    auto fileAndLine = ee.getFileAndLine();
    std::string msg =
      (fileAndLine.first.empty())
      ? ee.getMessage()
      : folly::format("{} in {} on line {}",
                      ee.getMessage(), fileAndLine.first,
                      fileAndLine.second).str();
    if (RuntimeOption::AutoloadRethrowExceptions) {
      throw;
    }
    log_err(msg.c_str());
    err = msg;
  } catch (Exception& e) {
    auto msg = e.getMessage();
    if (RuntimeOption::AutoloadRethrowExceptions) {
      throw;
    }
    log_err(msg.c_str());
    err = msg;
  } catch (Object& e) {
    log_err(e.toString().c_str());
    err = e;
  } catch (...) {
    String msg = "Unknown exception";
    log_err(msg.c_str());
    err = msg;
  }
  if (ok && checkExists()) {
    onPostAutoload(kind, clsName);
    return true;
  }
  return false;
}

template <class T>
bool
AutoloadHandler::loadFromMap(const String& clsName,
                             AutoloadMap::KindOf kind,
                             const T &checkExists) {
  assertx(m_map);
  Variant err{Variant::NullInit()};
  return loadFromMapImpl(clsName, kind, checkExists, err);
}

bool AutoloadHandler::autoloadFunc(StringData* name) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};
  return m_map &&
    loadFromMap(String{name},
                AutoloadMap::KindOf::Function,
                FuncExistsChecker(name));
}

bool AutoloadHandler::autoloadConstant(StringData* name) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};
  return m_map &&
    loadFromMap(String{name},
                AutoloadMap::KindOf::Constant,
                ConstExistsChecker(name));
}

bool AutoloadHandler::autoloadTypeAlias(const String& name) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};
  return m_map &&
    loadFromMap(name, AutoloadMap::KindOf::TypeAlias,
                TypeAliasExistsChecker(name));
}

bool AutoloadHandler::autoloadModule(StringData* name) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};
  return m_map &&
    loadFromMap(String{name},
                AutoloadMap::KindOf::Module,
                ModuleExistsChecker(name));
}

/**
 * Taken from php-src
 * https://github.com/php/php-src/blob/PHP-5.6/Zend/zend_execute_API.c#L960
 */
bool is_valid_class_name(folly::StringPiece className) {
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
  ) == className.size();
}

bool AutoloadHandler::autoloadType(const String& clsName) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};
  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);
  // Verify class name before trying to load it
  if (!is_valid_class_name(className.slice())) {
    return false;
  }
  return m_map &&
    loadFromMap(className, AutoloadMap::KindOf::Type,
                ClassExistsChecker(className));
}

bool AutoloadHandler::autoloadTypeOrTypeAlias(const String& clsName) {
  tracing::BlockNoTrace _{(m_map) ? "autoload-native" : "autoload"};

  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);

  return m_map &&
    loadFromMap(className, AutoloadMap::KindOf::TypeOrTypeAlias,
                NamedTypeExistsChecker(className));
}

void AutoloadHandler::setRepoAutoloadMap(std::unique_ptr<RepoAutoloadMap> map) {
  assertx(RO::RepoAuthoritative);
  assertx(!s_repoAutoloadMap);
  s_repoAutoloadMap = std::move(map);
}

void AutoloadHandler::setPostAutoloadHandler(Variant onPostAutoloadFunc) {
  always_assert(!RO::RepoAuthoritative);
  always_assert(
    onPostAutoloadFunc.isFunc() ||
    onPostAutoloadFunc.isObject() ||
    onPostAutoloadFunc.isClsMeth());
  m_onPostAutoloadFunc = std::move(onPostAutoloadFunc);
}

void AutoloadHandler::onPostAutoload(
    AutoloadMap::KindOf kind, const String& clsName) {
  if (m_onPostAutoloadFunc.isNull()) return;
  // We're merging type aliases into types in the autoloader
  switch (kind) {
    case AutoloadMap::KindOf::Type:
    case AutoloadMap::KindOf::TypeAlias:
    case AutoloadMap::KindOf::TypeOrTypeAlias:
      kind = AutoloadMap::KindOf::Type;
      break;
    default:
      break;
  }
  Variant action = vm_call_user_func(
    m_onPostAutoloadFunc,
    make_vec_array(Variant{static_cast<uint8_t>(kind)}, clsName));
}

//////////////////////////////////////////////////////////////////////

}
