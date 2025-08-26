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

#include "hphp/util/low-ptr-def.h"
#include "hphp/util/ptr-impl.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
using SmallPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32>;

template <typename T>
using UninitSmallPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32, false>;

template <typename T>
using AtomicSmallPtr = ptrimpl::PtrImpl<T, ptrimpl::Atomic, ptrimpl::UInt32>;

template <typename T>
using FullPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt64>;

template <typename T>
using UninitFullPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt64, false>;

template <typename T>
using AtomicFullPtr = ptrimpl::PtrImpl<T, ptrimpl::Atomic, ptrimpl::UInt64>;

#ifdef USE_LOWPTR

#ifdef USE_PACKEDPTR

template <typename T>
using PackedPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32Packed>;

template <typename T>
using UninitPackedPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32Packed, false>;

template <typename T>
using AtomicPackedPtr = ptrimpl::PtrImpl<T, ptrimpl::Atomic, ptrimpl::UInt32Packed>;

#else

template <typename T>
using PackedPtr = SmallPtr<T>;

template <typename T>
using UninitPackedPtr = UninitSmallPtr<T>;

template <typename T>
using AtomicPackedPtr = AtomicSmallPtr<T>;

#endif

#else

template <typename T>
using PackedPtr = FullPtr<T>;

template <typename T>
using UninitPackedPtr = UninitFullPtr<T>;

template <typename T>
using AtomicPackedPtr = AtomicFullPtr<T>;

#endif

///////////////////////////////////////////////////////////////////////////////

}
