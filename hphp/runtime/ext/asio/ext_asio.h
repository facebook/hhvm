/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_ASIO_H_
#define incl_HPHP_EXT_ASIO_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {

class AsioExtension final : public Extension {
public:
  AsioExtension() : Extension("asio", "0.1") {}

  void moduleInit() override {
    initFunctions();

    initWaitHandle();
    initWaitableWaitHandle();
    initResumableWaitHandle();
    initAsyncGenerator();
    initAwaitAllWaitHandle();
    initGenArrayWaitHandle();
    initGenMapWaitHandle();
    initGenVectorWaitHandle();
    initConditionWaitHandle();
    initSleepWaitHandle();
    initRescheduleWaitHandle();
    initExternalThreadEventWaitHandle();

    loadSystemlib();

    finishClasses();
  }

private:
  void initFunctions();

  void initWaitHandle();
  void initWaitableWaitHandle();
  void initResumableWaitHandle();
  void initAsyncGenerator();
  void initAwaitAllWaitHandle();
  void initGenArrayWaitHandle();
  void initGenMapWaitHandle();
  void initGenVectorWaitHandle();
  void initConditionWaitHandle();
  void initSleepWaitHandle();
  void initRescheduleWaitHandle();
  void initExternalThreadEventWaitHandle();

  void finishClasses();
};

Object HHVM_FUNCTION(asio_get_running);
size_t asio_object_size(const ObjectData* obj);

}

#endif // incl_HPHP_EXT_ASIO_H_
