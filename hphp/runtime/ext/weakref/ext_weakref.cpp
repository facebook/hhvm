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

#include "hphp/runtime/ext/weakref/weakref-data-handle.h"

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/weakref-data.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

const StaticString s_WeakRef("WeakRef");

WeakRefDataHandle& WeakRefDataHandle::operator=(
    const WeakRefDataHandle& other) {
  auto old_wr_data = wr_data;
  auto old_acquire_count = acquire_count;
  wr_data = other.wr_data;
  acquire_count = other.acquire_count;

  if (acquire_count > 0 && wr_data->isValid()) {
    tvIncRefCountable(wr_data->pointee);
  }
  if (old_acquire_count > 0 && old_wr_data->isValid()) {
    tvDecRefCountable(&(old_wr_data->pointee));
  }
  return *this;
}


WeakRefDataHandle::~WeakRefDataHandle() {
  if (acquire_count > 0 && wr_data->isValid()) {
    tvDecRefCountable(&(wr_data->pointee));
  }
}

namespace {

void HHVM_METHOD(WeakRef, __construct, const Variant& pointee) {
  if (UNLIKELY(!pointee.isObject())) {
    SystemLib::throwInvalidArgumentExceptionObject(
        "Parameter must be object type.");
  }

  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);

  wr_data_handle->wr_data =
    WeakRefData::forObject(pointee.toObject());
  wr_data_handle->acquire_count = 0;
}

bool HHVM_METHOD(WeakRef, acquire) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  if (LIKELY(wr_data_handle->wr_data->isValid())) {
    wr_data_handle->acquire_count++;
    if (wr_data_handle->acquire_count == 1) {
      tvIncRefCountable(wr_data_handle->wr_data->pointee);
    }
    assertx(wr_data_handle->acquire_count > 0);
    return true;
  }
  return false;
}

TypedValue HHVM_METHOD(WeakRef, get) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  if (wr_data_handle->wr_data->isValid()) {
    tvIncRefCountable(wr_data_handle->wr_data->pointee);
    return (wr_data_handle->wr_data->pointee);
  } else {
    return make_tv<KindOfNull>();
  }
}

bool HHVM_METHOD(WeakRef, release) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  if (LIKELY(wr_data_handle->wr_data->isValid()
        && wr_data_handle->acquire_count > 0)) {
    wr_data_handle->acquire_count--;
    if (wr_data_handle->acquire_count == 0) {
      tvDecRefCountable(&(wr_data_handle->wr_data->pointee));
    }
    return true;
  }
  return false;
}

bool HHVM_METHOD(WeakRef, valid) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  return wr_data_handle->wr_data->isValid();
}

///////////////////////////////////////////////////////////////////////////////
// Extension
struct WeakRefExtension final : Extension {
  WeakRefExtension() : Extension("weakref", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_ME(WeakRef, __construct);
    HHVM_ME(WeakRef, acquire);
    HHVM_ME(WeakRef, get);
    HHVM_ME(WeakRef, release);
    HHVM_ME(WeakRef, valid);

    Native::registerNativeDataInfo<WeakRefDataHandle>(s_WeakRef.get());
  }
} s_weakref_extension;

} // anonymous namespace
} // namespace HPHP
