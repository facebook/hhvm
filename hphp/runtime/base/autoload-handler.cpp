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

#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/repo-autoload-map.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

IMPLEMENT_REQUEST_LOCAL(AutoloadHandler, AutoloadHandler::s_instance);

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
  auto* factory = FactsFactory::getInstance();
  if (!factory) {
    return nullptr;
  }

  if (g_context.isNull()) {
    return nullptr;
  }

  auto* repoOptions = g_context->getRepoOptionsForRequest();
  if (!repoOptions) {
    return nullptr;
  }


  auto* map = factory->getForOptions(*repoOptions);
  if (!map) {
    return nullptr;
  }

  try {
    tracing::Block _{"autoload-ensure-updated"};
    map->ensureUpdated();
    return map;
  } catch (const std::exception& e) {
    auto repoRoot = folly::fs::canonical(repoOptions->path()).parent_path();
    Logger::Info(
        "Failed to update native autoloader, not natively autoloading %s. %s\n",
        repoRoot.generic().c_str(),
        e.what());
  }
  return nullptr;
}

} // namespace

void AutoloadHandler::requestInit() {
  assertx(!m_map);
  assertx(!m_facts);
  assertx(!m_req_map);
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
  m_req_map = nullptr;
}

bool AutoloadHandler::setMap(const Array& map, String root) {
  assertx(!RuntimeOption::RepoAuthoritative);

  m_req_map = req::make_unique<UserAutoloadMap>(
      UserAutoloadMap::fromFullMap(map, std::move(root)));
  m_map = m_req_map.get();
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
           (f->arFuncPtr() != Native::unimplementedWrapper);
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
    return type(Unit::lookupCns(m_name)) != KindOfUninit;
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

struct NamedTypeExistsChecker {
  const String& m_name;
  mutable NamedEntity* m_ne;
  explicit NamedTypeExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name.get(), false);
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedClass() != nullptr ||
           m_ne->getCachedTypeAlias() != nullptr ||
           m_ne->getCachedRecordDesc() != nullptr;
  }
};
struct RecordExistsChecker {
  const String& m_name;
  mutable NamedEntity* m_ne;
  explicit RecordExistsChecker(const String& name)
    : m_name(name), m_ne(nullptr) {}
  bool operator()() const {
    if (!m_ne) {
      m_ne = NamedEntity::get(m_name.get(), false);
      if (!m_ne) {
        return false;
      }
    }
    return m_ne->getCachedRecordDesc() != nullptr;
  }
};
}

const StaticString
  s_file("file"),
  s_line("line");

Optional<String> AutoloadHandler::getFile(const String& clsName,
                                               AutoloadMap::KindOf kind) {
  assertx(m_map);
  // Always normalize name before autoloading
  return m_map->getFile(kind, normalizeNS(clsName));
}

Array AutoloadHandler::getSymbols(const String& path,
                                  AutoloadMap::KindOf kind) {
  assertx(m_map);
  return m_map->getSymbols(kind, path);
}

template <class T>
AutoloadMap::Result
AutoloadHandler::loadFromMapImpl(const String& clsName,
                                 AutoloadMap::KindOf kind,
                                 const T &checkExists,
                                 Variant& err) {
  auto file = getFile(clsName, kind);
  if (!file) {
    return AutoloadMap::Result::Failure;
  }
  bool ok = false;
  String fName = *file;
  // Utility for logging errors in server mode.
  auto log_err = [](char const* const msg) {
    if (RuntimeOption::ServerMode) {
      Logger::Error("Exception: AutoloadMap::loadFromMapImpl: %s", msg);
    }
  };
  try {
    VMRegAnchor _;
    bool initial;
    auto const unit = lookupUnit(fName.get(), "", &initial,
                                 Native::s_noNativeFuncs,
                                 RuntimeOption::TrustAutoloaderPath);
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
    return AutoloadMap::Result::Success;
  }
  return AutoloadMap::Result::Failure;
}

template <class T>
AutoloadMap::Result
AutoloadHandler::loadFromMap(const String& clsName,
                             AutoloadMap::KindOf kind,
                             const T &checkExists) {
  assertx(m_map);
  while (true) {
    Variant err{Variant::NullInit()};
    AutoloadMap::Result res = loadFromMapImpl(clsName, kind, checkExists, err);
    if (res == AutoloadMap::Result::Success) {
      return AutoloadMap::Result::Success;
    }
    if (!m_map->canHandleFailure()) {
      return AutoloadMap::Result::Failure;
    }
    res = m_map->handleFailure(kind, clsName, err);
    if (checkExists()) return AutoloadMap::Result::Success;
    if (res == AutoloadMap::Result::RetryAutoloading) {
      continue;
    }
    return res;
  }
}

bool AutoloadHandler::autoloadFunc(StringData* name) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };
  return m_map &&
    loadFromMap(String{name},
                AutoloadMap::KindOf::Function,
                FuncExistsChecker(name)) != AutoloadMap::Result::Failure;
}

bool AutoloadHandler::autoloadConstant(StringData* name) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };
  return m_map &&
    loadFromMap(String{name},
                AutoloadMap::KindOf::Constant,
                ConstExistsChecker(name)) != AutoloadMap::Result::Failure;
}

bool AutoloadHandler::autoloadType(const String& name) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };
  return m_map &&
    loadFromMap(name, AutoloadMap::KindOf::TypeAlias,
                TypeExistsChecker(name)) != AutoloadMap::Result::Failure;
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

bool AutoloadHandler::autoloadClass(const String& clsName) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };
  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);
  // Verify class name before trying to load it
  if (!is_valid_class_name(className.slice())) {
    return false;
  }
  if (!m_map) {
    return false;
  }
  ClassExistsChecker ce(className);
  AutoloadMap::Result res = loadFromMap(className, AutoloadMap::KindOf::Type,
                                        ce);
  if (res == AutoloadMap::Result::Success || ce()) return true;
  return false;
}

bool AutoloadHandler::autoloadRecordDesc(const String& recName) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };
  if (recName.empty()) return false;
  return m_map &&
         loadFromMap(recName, AutoloadMap::KindOf::Type,
                     RecordExistsChecker(recName)) !=
         AutoloadMap::Result::Failure;
}

template<>
bool AutoloadHandler::autoloadType<Class>(const String& name) {
  return autoloadClass(name);
}
template<>
bool AutoloadHandler::autoloadType<RecordDesc>(const String& name) {
  return autoloadRecordDesc(name);
}

template <class T>
AutoloadMap::Result
AutoloadHandler::loadFromMapPartial(const String& className,
                                    AutoloadMap::KindOf kind,
                                    const T &checkExists,
                                    Variant& err) {
  AutoloadMap::Result res = loadFromMapImpl(className, kind, checkExists, err);
  if (res == AutoloadMap::Result::Success) {
    return AutoloadMap::Result::Success;
  }
  assertx(res == AutoloadMap::Result::Failure);
  if (!err.isNull()) {
    if (m_map->canHandleFailure()) {
      res = m_map->handleFailure(kind, className, err);
      assertx(res != AutoloadMap::Result::Failure);
      if (checkExists()) {
        return AutoloadMap::Result::Success;
      }
    }
  }
  return res;
}

bool AutoloadHandler::autoloadNamedType(const String& clsName) {
  tracing::BlockNoTrace _{
    (m_map && m_map->isNative()) ? "autoload-native" : "autoload"
  };

  if (clsName.empty()) return false;
  const String& className = normalizeNS(clsName);
  if (!m_map) {
    return false;
  }

  NamedTypeExistsChecker cte(className);
  bool tryType = true, tryTypeAlias = true;
  AutoloadMap::Result typeRes = AutoloadMap::Result::RetryAutoloading,
                      typeAliasRes = AutoloadMap::Result::RetryAutoloading;
  while (true) {
    Variant typeErr{Variant::NullInit()};
    if (tryType) {
      // Try consulting the 'type' map first, but don't call the failure
      // callback unless there was an uncaught exception or a fatal error
      // during the include operation.
      typeRes = loadFromMapPartial(className, AutoloadMap::KindOf::Type, cte,
                                   typeErr);
      if (typeRes == AutoloadMap::Result::Success) return true;
    }
    Variant typeAliasErr{Variant::NullInit()};
    if (tryTypeAlias) {
      // Next, try consulting the 'type alias' map. Again, don't call the
      // failure callback unless there was an uncaught exception
      // or fatal error.
      typeAliasRes = loadFromMapPartial(className,
                                        AutoloadMap::KindOf::TypeAlias, cte,
                                        typeAliasErr);
      if (typeAliasRes == AutoloadMap::Result::Success) return true;
    }
    // If we reach this point, then for each map either nothing was found
    // or the file we included didn't define a class or type alias or record
    // with the specified name, and the failure callback (if one exists)
    // did not throw or raise a fatal error.
    if (m_map->canHandleFailure()) {
      // First, call the failure callback for 'class' if we didn't do so
      // above
      if (typeRes == AutoloadMap::Result::Failure) {
        assertx(tryType);
        typeRes = m_map->handleFailure(AutoloadMap::KindOf::Type,
                                        className, typeErr);
        // The failure callback may have defined a class or record for
        // us, in which case we're done.
        if (cte()) return true;
      }
      // Next, call the failure callback for 'type alias'
      // if we didn't do so above
      if (typeAliasRes == AutoloadMap::Result::Failure) {
        assertx(tryTypeAlias);
        typeAliasRes = m_map->handleFailure(AutoloadMap::KindOf::TypeAlias,
                                            className, typeAliasErr);
        // The failure callback may have defined a class or type alias for
        // us, in which case we're done.
        if (cte()) return true;
      }
      assertx(typeRes != AutoloadMap::Result::Failure &&
              typeAliasRes != AutoloadMap::Result::Failure);
      tryType = (typeRes == AutoloadMap::Result::RetryAutoloading);
      tryTypeAlias = (typeAliasRes == AutoloadMap::Result::RetryAutoloading);
      // If the failure callback requested a retry for 'class', 'type', or
      // 'record', jump back to the top to try again.
      if (tryType || tryTypeAlias) {
        continue;
      }
    }

    return false;
  }
}

void AutoloadHandler::setRepoAutoloadMap(std::unique_ptr<RepoAutoloadMap> map) {
  assertx(RO::RepoAuthoritative);
  assertx(!s_repoAutoloadMap);
  s_repoAutoloadMap = std::move(map);
}

//////////////////////////////////////////////////////////////////////

}
