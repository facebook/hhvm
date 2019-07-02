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
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
const StaticString
   s_FunctionCredential("FunctionCredential"),
   s_className("class_name"),
   s_functionName("function_name"),
   s_fileName("file_name");
}

// static
ObjectData* FunctionCredential::newInstance(const Func* func) {
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

// static
Class* FunctionCredential::classof() {
  static Class* cls_FunctionCredential =
    Unit::lookupClass(s_FunctionCredential.get());
  assertx(cls_FunctionCredential);
  return cls_FunctionCredential;
}

static Array HHVM_METHOD(FunctionCredential, __debugInfo) {
    auto data = FunctionCredential::fromObject(this_);

    String className;
    String functionName;
    String fileName;

    auto func = data->func();

    if (func) {
      functionName = makeStaticString(func->name());
      fileName = makeStaticString(func->filename());

      auto cls = func->cls();
      if (cls) {
         className = makeStaticString(cls->name());
      }
    }

    return make_darray(
      s_className,
      std::move(className),
      s_functionName,
      std::move(functionName),
      s_fileName,
      std::move(fileName));
}

namespace {
struct FunctionCredentialExtension final : Extension {
  FunctionCredentialExtension() : Extension("functioncredential", "1.0") {}

  void moduleInit() override {
    HHVM_ME(FunctionCredential, __debugInfo);

    Native::registerNativeDataInfo<FunctionCredential>(
      s_FunctionCredential.get());

    loadSystemlib();
  }
} s_functioncredential_extension;
}
}
