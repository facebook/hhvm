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

#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/ext/asio/ext_asio.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

rds::Link<Object, rds::Mode::Normal> c_StaticWaitHandle::NullHandle;
rds::Link<Object, rds::Mode::Normal> c_StaticWaitHandle::TrueHandle;
rds::Link<Object, rds::Mode::Normal> c_StaticWaitHandle::FalseHandle;

c_StaticWaitHandle* c_StaticWaitHandle::CreateSucceededImpl(const TypedValue result) {
  auto waitHandle = req::make<c_StaticWaitHandle>();
  waitHandle->setState(STATE_SUCCEEDED);
  tvCopy(result, waitHandle->m_resultOrException);
  return waitHandle.detach();
}

/**
 * Create succeeded StaticWaitHandle object.
 *
 * - consumes reference of the given cell
 * - produces reference for the returned StaticWaitHandle object
 *
 * Both the JIT and bytecode.cpp assume this function gives the nothrow
 * guarantee.
 */
c_StaticWaitHandle* c_StaticWaitHandle::CreateSucceeded(const TypedValue result) {
  if (isNullType(result.m_type)) {
    Object ret = *NullHandle.get();
    return static_cast<c_StaticWaitHandle*>(ret.detach());
  }
  if (isBoolType(result.m_type)) {
    Object ret = result.m_data.num ? *TrueHandle.get() : *FalseHandle.get();
    return static_cast<c_StaticWaitHandle*>(ret.detach());
  }
  return CreateSucceededImpl(result);
}

/**
 * Create failed StaticWaitHandle object.
 *
 * - consumes reference of the given Exception object
 * - produces reference for the returned StaticWaitHandle object
 */
c_StaticWaitHandle* c_StaticWaitHandle::CreateFailed(ObjectData* exception) {
  assertx(exception);
  assertx(exception->instanceof(SystemLib::getThrowableClass()));

  auto waitHandle = req::make<c_StaticWaitHandle>();
  waitHandle->setState(STATE_FAILED);
  tvCopy(make_tv<KindOfObject>(exception), waitHandle->m_resultOrException);
  return waitHandle.detach();
}

void AsioExtension::initStaticWaitHandle() {
  c_StaticWaitHandle::NullHandle.bind(
    rds::Mode::Normal, rds::LinkID{"StaticNullWH"});
  c_StaticWaitHandle::TrueHandle.bind(
    rds::Mode::Normal, rds::LinkID{"StaticTrueWH"});
  c_StaticWaitHandle::FalseHandle.bind(
    rds::Mode::Normal, rds::LinkID{"StaticFalseWH"});

  Native::registerClassExtraDataHandler(
    c_StaticWaitHandle::className(), finish_class<c_StaticWaitHandle>);
}

void AsioExtension::requestInitSingletons() {
  using SSWH = c_StaticWaitHandle;
  Object nullObj{SSWH::CreateSucceededImpl(make_tv<KindOfNull>())};
  Object trueObj{SSWH::CreateSucceededImpl(make_tv<KindOfBoolean>(true))};
  Object falseObj{SSWH::CreateSucceededImpl(make_tv<KindOfBoolean>(false))};

  SSWH::NullHandle.initWith(std::move(nullObj));
  SSWH::TrueHandle.initWith(std::move(trueObj));
  SSWH::FalseHandle.initWith(std::move(falseObj));
}

///////////////////////////////////////////////////////////////////////////////
}
