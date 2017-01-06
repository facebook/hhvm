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

#ifndef incl_HPHP_EXT_WEAKREF_H_
#define incl_HPHP_EXT_WEAKREF_H_

#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/weakref-data.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s_WeakRefDataHandle("WeakRefDataHandle");
struct WeakRefDataHandle final {
  // We share the general validity and pointer between WeakRefHandles.
  req::shared_ptr<WeakRefData> wr_data;
  int32_t acquire_count;

  WeakRefDataHandle(const WeakRefDataHandle&) = delete;

  WeakRefDataHandle(): wr_data(nullptr), acquire_count(0) {}
  WeakRefDataHandle& operator=(const WeakRefDataHandle& other) {
    auto old_wr_data = wr_data;
    auto old_acquire_count = acquire_count;
    wr_data = other.wr_data;
    acquire_count = other.acquire_count;

    if (acquire_count > 0 && wr_data->pointee.m_type != KindOfUninit) {
      tvIncRef(&(wr_data->pointee));
    }
    if (old_acquire_count > 0 && old_wr_data->pointee.m_type != KindOfUninit) {
      tvDecRef(&(old_wr_data->pointee));
    }
    return *this;
  }

  void sweep() {}

  ~WeakRefDataHandle() {
    if (acquire_count > 0 && wr_data->pointee.m_type != KindOfUninit) {
      tvDecRef(&(wr_data->pointee));
    }
  }
};

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
  if (LIKELY(wr_data_handle->wr_data->pointee.m_type != KindOfUninit)) {
    wr_data_handle->acquire_count++;
    if (wr_data_handle->acquire_count == 1) {
      tvIncRef(&(wr_data_handle->wr_data->pointee));
    }
    assert(wr_data_handle->acquire_count > 0);
    return true;
  }
  return false;
}

TypedValue HHVM_METHOD(WeakRef, get) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  if (wr_data_handle->wr_data->pointee.m_type != KindOfUninit) {
    tvIncRef(&(wr_data_handle->wr_data->pointee));
    return (wr_data_handle->wr_data->pointee);
  } else {
    return make_tv<KindOfNull>();
  }
}

bool HHVM_METHOD(WeakRef, release) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  if (LIKELY(wr_data_handle->wr_data->pointee.m_type != KindOfUninit
        && wr_data_handle->acquire_count > 0)) {
    wr_data_handle->acquire_count--;
    if (wr_data_handle->acquire_count == 0) {
      tvDecRef(&(wr_data_handle->wr_data->pointee));
    }
    return true;
  }
  return false;
}

bool HHVM_METHOD(WeakRef, valid) {
  auto wr_data_handle = Native::data<WeakRefDataHandle>(this_);
  return wr_data_handle->wr_data->pointee.m_type != KindOfUninit;
}

///////////////////////////////////////////////////////////////////////////////
// Extension
struct WeakRefExtension final : Extension {
  WeakRefExtension() : Extension("weakref") {}
  void moduleInit() override {
    HHVM_ME(WeakRef, __construct);
    HHVM_ME(WeakRef, acquire);
    HHVM_ME(WeakRef, get);
    HHVM_ME(WeakRef, release);
    HHVM_ME(WeakRef, valid);

    Native::registerNativeDataInfo<WeakRefDataHandle>(
      s_WeakRefDataHandle.get());

    loadSystemlib("weakref");
  }
} s_weakref_extension;

} // anonymous namespace
} // namespace HPHP
#endif // incl_HPHP_EXT_WEAKREF_H_
