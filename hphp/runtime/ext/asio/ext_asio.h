/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

struct AsioExtension final : Extension {
  AsioExtension() : Extension("asio", "0.1", NO_ONCALL_YET) {}
  void moduleInit() override;
  void moduleRegisterNative() override;
  void requestInit() override;
  std::vector<std::string> hackFiles() const override;

private:
  void registerNativeFunctions();

  void registerNativeWaitHandle();
  void registerNativeResumableWaitHandle();
  void registerNativeAsyncGenerator();
  void registerNativeAwaitAllWaitHandle();
  void registerNativeConcurrentWaitHandle();
  void registerNativeConditionWaitHandle();
  void registerNativeSleepWaitHandle();
  void registerNativeRescheduleWaitHandle();
  void registerNativeExternalThreadEventWaitHandle();
  void registerNativeStaticWaitHandle();
  void registerNativeAsyncFunctionWaitHandle();
  void registerNativeAsyncGeneratorWaitHandle();

  void initNativeWaitHandle();

  void requestInitSingletons();

};

Object HHVM_FUNCTION(asio_get_running);
Variant HHVM_FUNCTION(join, const Object& obj);
size_t asio_object_size(const ObjectData* obj);

}

#endif // incl_HPHP_EXT_ASIO_H_
