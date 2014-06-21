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

#include "hphp/runtime/base/smart-containers.h"
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
    ContinueAutoloading
  };

  struct HandlerBundle {
    HandlerBundle() = delete;
    HandlerBundle(const Variant& handler,
                  smart::unique_ptr<CufIter>& cufIter) :
      m_handler(handler) {
      m_cufIter = std::move(cufIter);
    }

    Variant m_handler; // used to respond to f_spl_autoload_functions
    smart::unique_ptr<CufIter> m_cufIter; // used to invoke handlers
  };

  class CompareBundles {
public:
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

  virtual void requestInit();
  virtual void requestShutdown();

  Array getHandlers();
  bool addHandler(const Variant& handler, bool prepend);
  void removeHandler(const Variant& handler);
  void removeAllHandlers();
  bool isRunning();

  bool invokeHandler(const String& className, bool forceSplStack = false);
  bool autoloadFunc(StringData* name);
  bool autoloadConstant(StringData* name);
  bool autoloadType(const String& name);
  bool setMap(const Array& map, const String& root);
  DECLARE_STATIC_REQUEST_LOCAL(AutoloadHandler, s_instance);

private:
  template <class T>
  Result loadFromMap(const String& name, const String& kind, bool toLower,
                     const T &checkExists);
  static String getSignature(const Variant& handler);

  Array m_map;
  String m_map_root;
  bool m_spl_stack_inited;
  union {
    smart::deque<HandlerBundle> m_handlers;
  };
  Array m_loading;
};

//////////////////////////////////////////////////////////////////////

}

#endif
