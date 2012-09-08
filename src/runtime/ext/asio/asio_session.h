/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_ASIO_SESSION_H__
#define __EXT_ASIO_SESSION_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AsioContext;
FORWARD_DECLARE_CLASS_BUILTIN(Closure);
FORWARD_DECLARE_CLASS_BUILTIN(Exception);

class AsioSession {
  public:
    static void Init();

    // context
    static inline AsioContext* GetCurrentContext() { return s_ctx.get(); }
    static inline void SetCurrentContext(AsioContext* ctx) { s_ctx.set(ctx); }

    // callback: on failed
    static void SetOnFailedCallback(CObjRef on_failed_cb);
    static void OnFailed(CObjRef exception);


  private:
    static DECLARE_THREAD_LOCAL_PROXY(AsioContext, false, s_ctx);
    static DECLARE_THREAD_LOCAL_PROXY(ObjectData, false, s_on_failed_cb);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ASIO_SESSION_H__
