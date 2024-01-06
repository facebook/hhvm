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
#pragma once

#include <memory>
#include <utility>

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/req-deque.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/util/rds-local.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool is_valid_class_name(folly::StringPiece className);

struct RepoAutoloadMap;

struct AutoloadHandler final : RequestEventHandler {

  AutoloadHandler() = default;
  AutoloadHandler(const AutoloadHandler&) = delete;
  AutoloadHandler(AutoloadHandler&&) = delete;
  AutoloadHandler& operator=(const AutoloadHandler&) = delete;
  AutoloadHandler& operator=(AutoloadHandler&&) noexcept = delete;
  ~AutoloadHandler() = default;

  void requestInit() override;
  void requestShutdown() override;

  /**
   * autoloadTypeOrTypeAlias() tries to autoload either a type or a type
   * alias with the specified name.
   */
  bool autoloadTypeOrTypeAlias(const String& className);

  bool autoloadType(const String& className);
  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadTypeAlias(const String& name);
  bool autoloadModule(StringData* name);
  static RDS_LOCAL(AutoloadHandler, s_instance);

  const AutoloadMap* getAutoloadMap() const {
    return m_map;
  }

  AutoloadMap* getAutoloadMap() {
    return m_map;
  }

  FactsStore* getFacts() {
    return m_facts;
  }

  Optional<AutoloadMap::FileResult> getFile(const String& name,
                                            AutoloadMap::KindOf kind);

  static void setRepoAutoloadMap(std::unique_ptr<RepoAutoloadMap>);

private:
  /**
   * This method may return true on success or false on failure.
   */
  template <class T>
  bool loadFromMapImpl(const String& name, AutoloadMap::KindOf kind,
                       const T &checkExists, Variant& err);

  /**
   * This method attempts to load the unit containing the given symbol,
   * and will return true on success.
   */
  template <class T>
  bool loadFromMap(const String& name, AutoloadMap::KindOf kind,
                   const T &checkExists);

private:

  // The value of m_map determines which data structure, if any, we'll be
  // using for autoloading within this request. m_map may have the same value
  // as m_facts, a statically-scoped native AutoloadMap that can answer
  // queries (aka Facts) about the codebase.
  FactsStore* m_facts = nullptr;
  AutoloadMap* m_map = nullptr;

  static std::unique_ptr<RepoAutoloadMap> s_repoAutoloadMap;
};

//////////////////////////////////////////////////////////////////////

/**
 * Set this inside of an extension's moduleLoad() to provide an
 * implementation for a native AutoloadMap/Facts.
 */
struct FactsFactory {

  static FactsFactory* getInstance();
  static void setInstance(FactsFactory* instance);

  FactsFactory() = default;
  FactsFactory(const FactsFactory&) = default;
  FactsFactory(FactsFactory&&) noexcept = default;
  FactsFactory& operator=(const FactsFactory&) = default;
  FactsFactory& operator=(FactsFactory&&) noexcept = default;
  virtual ~FactsFactory() = default;

  /**
   * Return a Facts corresponding to the given options. If one doesn't exist
   * yet, create it.
   */
  virtual FactsStore* getForOptions(const RepoOptions& options) = 0;
};

}
