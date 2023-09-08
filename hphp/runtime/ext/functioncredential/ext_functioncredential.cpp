// Copyright 2004-present Facebook. All Rights Reserved.

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

#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// static
ObjectData* FunctionCredential::newInstance(const Func* func) {
  assertx(func);
  auto objData = ObjectData::newInstance(classof());
  auto data = Native::data<FunctionCredential>(objData);
  data->func_ = func;
  return objData;
}

// static
const FunctionCredential* FunctionCredential::fromObject(
    const ObjectData* obj) {
  return Native::data<FunctionCredential>(obj);
}

static TypedValue HHVM_METHOD(FunctionCredential, getClassName) {
  auto data = FunctionCredential::fromObject(this_);
  auto func = data->func();

  auto cls = func->cls();
  return cls ? make_tv<KindOfPersistentString>(cls->name())
             : make_tv<KindOfNull>();
}

static String HHVM_METHOD(FunctionCredential, getFunctionName) {
  auto data = FunctionCredential::fromObject(this_);
  return String{makeStaticString(data->func()->name())};
}

static String HHVM_METHOD(FunctionCredential, getFilename) {
  auto data = FunctionCredential::fromObject(this_);
  return String{makeStaticString(data->func()->filename())};
}

namespace {
struct FunctionCredentialExtension final : Extension {
  FunctionCredentialExtension() : Extension("functioncredential", "1.0", NO_ONCALL_YET) {}

  void moduleInit() override {
    HHVM_ME(FunctionCredential, getClassName);
    HHVM_ME(FunctionCredential, getFunctionName);
    HHVM_ME(FunctionCredential, getFilename);

    Native::registerNativeDataInfo<FunctionCredential>();
  }
} s_functioncredential_extension;
} // namespace
} // namespace HPHP
