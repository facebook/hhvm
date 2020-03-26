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
#ifndef incl_HPHP_AUTOLOAD_HANDLER_H_
#define incl_HPHP_AUTOLOAD_HANDLER_H_

#include <memory>
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

struct AutoloadMapFactory;

struct AutoloadHandler final : RequestEventHandler {
  struct DecodedHandler {
    DecodedHandler(ObjectData* obj, Class* cls, const Func* func,
                   StringData* name, bool dynamic) :
      m_obj(obj), m_cls(cls), m_func(func), m_name(name), m_dynamic(dynamic) {
      assertx(!m_obj || !m_cls);
    }

    Object m_obj;
    Class* m_cls;
    const Func* m_func;
    String m_name;
    bool m_dynamic;
  };

private:

  struct HandlerBundle {
    HandlerBundle() = delete;
    HandlerBundle(const Variant& handler,
                  req::unique_ptr<DecodedHandler>& decodedHandler) :
      m_handler(handler) {
      m_decodedHandler = std::move(decodedHandler);
    }
    Variant m_handler; // used to respond to f_spl_autoload_functions
    req::unique_ptr<DecodedHandler> m_decodedHandler; // used to invoke handlers
  };

  struct CompareBundles {
    explicit CompareBundles(DecodedHandler* h) : m_decodedHandler(h) { }
    bool operator()(const HandlerBundle& hb);
   private:
    DecodedHandler* m_decodedHandler;
  };

public:
  AutoloadHandler() { }

  ~AutoloadHandler() {
    // m_handlers won't run a destructor so nothing to do here
    m_loading.detach();
  }

  void requestInit() override;
  void requestShutdown() override;

  Array getHandlers();
  bool addHandler(const Variant& handler, bool prepend);
  void removeHandler(const Variant& handler);
  void removeAllHandlers();
  bool isRunning();

  template<class T>
  bool autoloadType(const String& name);

  bool autoloadClass(const String& className, bool forceSplStack = false);
  bool autoloadClassPHP5Impl(const String& className, bool forceSplStack);

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

  folly::Optional<String> getFile(const String& name,
                                  AutoloadMap::KindOf kind,
                                  bool toLower);

  Array getSymbols(const String& path, AutoloadMap::KindOf kind);

private:
  /**
   * This method may return Success or Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMapImpl(const String& name,
                                      AutoloadMap::KindOf kind,
                                      bool toLower,
                                      const T &checkExists,
                                      Variant& err);

  /**
   * This method may return ContinueAutoloading, StopAutoloading, or
   * RetryAutoloading.
   */
  AutoloadMap::Result invokeFailureCallback(const_variant_ref func,
                                            AutoloadMap::KindOf kind,
                                            const String& name,
                                            const Variant& err);

  /**
   * loadFromMap() will call the failure callback if the specified name is not
   * present in the specified map, or if there is an entry in the map but there
   * was an error during the include operation. loadFromMap() will also retry
   * loading the specified name from the map if the failure callback returned
   * boolean true. Note that calling this method may throw if the failure
   * callback throws an exception or raises a fatal error.
   *
   * This method may return Success, Failure, ContinueAutoloading, or
   * StopAutoloading. If the failure callback was called, this method will not
   * return Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMap(const String& name, AutoloadMap::KindOf kind,
                                  bool toLower, const T &checkExists);

  /**
   * loadFromMapPartial() will call the failure callback if there is an error
   * during the include operation, but otherwise it will not call the failure
   * callback.
   *
   * This method may return Success, Failure, ContinueAutoloading,
   * StopAutoloading, or RetryAutoloading. If the failure callback was called,
   * this method will not return Failure.
   */
  template <class T>
  AutoloadMap::Result loadFromMapPartial(const String& className,
                                         AutoloadMap::KindOf kind, bool toLower,
                                         const T &checkExists, Variant& err);

  static String getSignature(const Variant& handler);

private:

  // m_map points to either the request-scoped userland AutoloadMap or
  // a statically-scoped native AutoloadMap
  AutoloadMap* m_map = nullptr;
  req::unique_ptr<UserAutoloadMap> m_req_map;
  bool m_spl_stack_inited{false};
  union {
    req::deque<HandlerBundle> m_handlers;
  };
  bool m_handlers_valid{false};
  Array m_loading;
  TYPE_SCAN_CUSTOM_FIELD(m_handlers) {
    if (m_handlers_valid) { scanner.scan(m_handlers); }
  }
};

//////////////////////////////////////////////////////////////////////

/**
 * Set this inside of an extension's moduleLoad() to provide an
 * implementation for a native AutoloadMap.
 */
struct AutoloadMapFactory {

  static AutoloadMapFactory* getInstance();
  static void setInstance(AutoloadMapFactory* instance);

  virtual ~AutoloadMapFactory() = default;

  /**
   * Return an AutoloadMap corresponding to the given root. If one
   * doesn't exist yet, create it.
   */
  virtual AutoloadMap* getForRoot(const folly::fs::path& root) = 0;
};

}

#endif
