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
#ifndef incl_HPHP_AUTOLOAD_HANDLER_H_
#define incl_HPHP_AUTOLOAD_HANDLER_H_

#include <utility>

#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

class AutoloadHandler final : public RequestEventHandler {
  enum Result {
    Failure,
    Success,
    StopAutoloading,
    ContinueAutoloading,
    RetryAutoloading
  };

  struct HandlerBundle {
    HandlerBundle() = delete;
    HandlerBundle(const Variant& handler,
                  req::unique_ptr<CufIter>& cufIter) :
      m_handler(handler) {
      m_cufIter = std::move(cufIter);
    }

    Variant m_handler; // used to respond to f_spl_autoload_functions
    req::unique_ptr<CufIter> m_cufIter; // used to invoke handlers
  };

  struct CompareBundles {
    explicit CompareBundles(CufIter* cufIter) : m_cufIter(cufIter) { }
    bool operator()(const HandlerBundle& hb);
   private:
    CufIter* m_cufIter;
  };

public:
  AutoloadHandler() { }

  ~AutoloadHandler() {
    m_map.detach();
    m_map_root.detach();
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

  bool autoloadClass(const String& className, bool forceSplStack = false);
  bool autoloadClassPHP5Impl(const String& className, bool forceSplStack);

  /**
   * autoloadClassOrType() tries to autoload either a class or a type alias
   * with the specified name. This method avoids calling the failure callback
   * until one of the following happens: (1) we tried to autoload the specified
   * name from both the 'class' and 'type' maps but for each map either nothing
   * was found or the file we included did not define a class or type alias
   * with the specified name, or (2) there was an uncaught exception or fatal
   * error during an include operation.
   */
  bool autoloadClassOrType(const String& className);

  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadType(const String& name);
  bool setMap(const Array& map, const String& root);
  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

private:
  /**
   * This method may return Success or Failure.
   */
  template <class T>
  Result loadFromMapImpl(const String& name, const String& kind, bool toLower,
                         const T &checkExists, Variant& err);

  /**
   * This method may return ContinueAutoloading, StopAutoloading, or
   * RetryAutoloading.
   */
  Result invokeFailureCallback(const Variant& func, const String& kind,
                               const String& name, const Variant& err);

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
  Result loadFromMap(const String& name, const String& kind, bool toLower,
                     const T &checkExists);

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
  Result loadFromMapPartial(const String& className, const String& kind,
                            bool toLower, const T &checkExists, Variant& err);

  static String getSignature(const Variant& handler);

  Array m_map;
  String m_map_root;
  bool m_spl_stack_inited;
  union {
    req::deque<HandlerBundle> m_handlers;
  };
  Array m_loading;
};

//////////////////////////////////////////////////////////////////////

}

#endif
