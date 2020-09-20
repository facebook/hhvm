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

#pragma once

#include "hphp/runtime/base/apc-handle.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/rfunc-data.h"

namespace HPHP {

/**
 * Representation of a rfunc stored in APC
 *
 * TODO: figure out something about serialized form - see comment in apc-object
 * where we would want to represent it by an APCString incstance with type KindOfRFunc
 */

struct APCRFunc {

  /*
   * Create an APCObject from an RFuncData*; returns its APCHandle.
   */
  static APCHandle::Pair Construct(RFuncData* data);

  const static APCRFunc* fromHandle(const APCHandle* handle);

  static Variant Make(const APCHandle* handle);

  static void Delete(APCHandle* handle);

  APCHandle* getHandle() { return &m_handle; }

private:
  APCRFunc(RFuncData* rfuncData, APCHandle* generics);

private:
  APCHandle m_handle;
  LowPtr<const NamedEntity> m_entity;
  LowPtr<const StringData> m_name;
public:
  APCHandle* m_generics;
};

} // namespace HPHP

