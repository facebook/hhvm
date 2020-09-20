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

namespace HPHP {

struct APCRClsMeth {

  static APCHandle::Pair Construct(RClsMethData* data);

  static Variant Make(const APCHandle* handle);

  static void Delete(APCHandle* handle);

  APCHandle* getHandle() { return &m_handle; }

  const static APCRClsMeth* fromHandle(const APCHandle* handle);

private:
  APCRClsMeth(RClsMethData* data, APCHandle* generics);

private:
  APCHandle m_handle;
  LowPtr<const StringData> m_cls_name;
  LowPtr<const StringData> m_func_name;
public:
  APCHandle* m_generics;
};

} // namespace HPHP

