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

#include "hphp/util/low-ptr.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {

struct Unit;
struct UnitEmitter;

struct LowStringPtrOrId {

  using storage_type = LowPtr<const StringData>::storage_type;

  LowStringPtrOrId(): m_s(0) {}
  explicit LowStringPtrOrId(const LowStringPtr str) : HPHP::LowStringPtrOrId(str.get()) {}
  explicit LowStringPtrOrId(const StringData* str);
  explicit LowStringPtrOrId(Id id);
  LowStringPtrOrId(const LowStringPtrOrId& other);

  LowStringPtrOrId& operator=(const StringData* str);
  LowStringPtrOrId& operator=(const LowStringPtrOrId& other);

  Id id() const;
  const StringData* rawPtr() const;
  const StringData* ptr(const Unit* unit) const;
  const StringData* ptr(const UnitEmitter& ue) const;

private:
  mutable std::atomic<storage_type> m_s;
};

template<>
struct BlobEncoderHelper<LowStringPtrOrId> {
  static void serde(BlobEncoder&, const LowStringPtrOrId&);
  static void serde(BlobDecoder&, LowStringPtrOrId&);
};

} // namespace HPHP
