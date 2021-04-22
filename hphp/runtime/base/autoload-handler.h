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
#include <optional>
#include <utility>

#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/req-deque.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/user-autoload-map.h"
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

  template<class T>
  bool autoloadType(const String& name);

  bool autoloadClass(const String& className);

  /**
   * autoloadNamedType() tries to autoload either a class or
   * a type alias or a record with the specified name. This method avoids
   * calling the failure callback until one of the following happens:
   * (1) we tried to autoload the specified name from the 'class', 'type' and
   * 'record' maps but for each map either nothing was found or the file
   * we included did not define a class or type alias or record
   * with the specified name, or (2) there was an uncaught exception or fatal
   * error during an include operation.
   */
  bool autoloadNamedType(const String& className);

  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadType(const String& name);
  bool autoloadRecordDesc(const String& name);
  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

  /**
   * Initialize the AutoloadHandler with a given root directory and map of
   * symbols to files.
   *
   * The map has the form:
   *
   * ```
   *  shape('class'    => dict['cls' => 'cls_file.php', ...],
   *        'function' => dict['fun' => 'fun_file.php', ...],
   *        'constant' => dict['con' => 'con_file.php', ...],
   *        'type'     => dict['type' => 'type_file.php', ...],
   *        'failure'  => (string $type, string $name, mixed $err): ?bool ==> {
   *          return null;  // KEEP_GOING We don't know where this symbol is,
   *                                      but it isn't important. Ignore the
   *                                      failure.
   *          return true;  // RETRY We require_once'd the correct file and the
   *                                 symbol should now be loaded. Try again.
   *          return false; // STOP We don't know where this symbol is and we
   *                                need to know where it is to correctly
   *                                continue the request. Abort the request.
   *        });
   * ```
   */
  bool setMap(const Array& map, String root);

  const AutoloadMap* getAutoloadMap() const {
    return m_map;
  }

  FactsStore* getFacts() {
    return m_facts;
  }

  std::optional<String> getFile(const String& name,
                                  AutoloadMap::KindOf kind);

  Array getSymbols(const String& path, AutoloadMap::KindOf kind);

  static void setRepoAutoloadMap(std::unique_ptr<RepoAutoloadMap>);

private:
  /**
   * This method may return Success or Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMapImpl(const String& name,
                                      AutoloadMap::KindOf kind,
                                      const T &checkExists,
                                      Variant& err);

  /**
   * loadFromMap() will call the failure callback if the specified name is not
   * present in the specified map, or if there is an entry in the map but there
   * was an error during the include operation. loadFromMap() will also retry
   * loading the specified name from the map if the failure callback returned
   * boolean true. Note that calling this method may throw if the failure
   * callback throws an exception or raises a fatal error.
   *
   * This method may return Success, Failure, or StopAutoloading. If the
   * failure callback was called, this method will not return Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMap(const String& name, AutoloadMap::KindOf kind,
                                  const T &checkExists);

  /**
   * loadFromMapPartial() will call the failure callback if there is an error
   * during the include operation, but otherwise it will not call the failure
   * callback.
   *
   * This method may return Success, Failure, StopAutoloading, or
   * RetryAutoloading. If the failure callback was called, this method will not
   * return Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMapPartial(const String& className,
                                         AutoloadMap::KindOf kind,
                                         const T &checkExists, Variant& err);

  static String getSignature(const Variant& handler);

private:

  // The value of m_map determines which data structure, if any, we'll be
  // using for autoloading within this request. m_map may have the same value
  // as m_req_map (a request-scoped AutoloadMap set from userland) or m_facts
  // (a statically-scoped native AutoloadMap that can answer additional
  // queries about the codebase).
  FactsStore* m_facts = nullptr;
  req::unique_ptr<UserAutoloadMap> m_req_map;
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
