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
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/vm/coeffects.h"

#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// Code should not use the values of these TypedValues, so we set them to
// unusual values that may (and, indeed, have) caught logic errors.
const TypedValue immutable_null_base{0xdeadbeef, KindOfNull};
const TypedValue immutable_uninit_base{0xfacade, KindOfUninit};

std::string TypedValue::pretty() const {
  char buf[20];
  snprintf(buf, sizeof(buf), "0x%lx", long(m_data.num));
  return Trace::prettyNode(tname(m_type).c_str(), std::string(buf));
}

StaticCoeffects ConstModifiers::getCoeffects() const {
  assertx(kind() == ConstModifiers::Kind::Context);
  return StaticCoeffects::fromValue(rawData >> ConstModifiers::kDataShift);
}

void ConstModifiers::setCoeffects(StaticCoeffects coeffects) {
  rawData = (uintptr_t)(coeffects.value() << ConstModifiers::kDataShift)
              | (rawData & ~ConstModifiers::kMask);
}

//////////////////////////////////////////////////////////////////////

void TypedValue::serde(BlobEncoder& encoder) const {
  if (m_type == KindOfUninit) {
    encoder(static_cast<uint32_t>(0));
    return;
  }
  auto const s = internal_serialize(tvAsCVarRef(this));
  assertx(s.get());
  auto const sz = s.size();
  encoder(static_cast<uint32_t>(sz));
  if (!sz) return;
  encoder.writeRaw(s.data(), sz);
}

void TypedValue::serde(BlobDecoder& decoder) {
  tvWriteUninit(*this);

  uint32_t size;
  decoder(size);
  if (size == 0) return;

  auto const data = decoder.data();
  std::string str{data, data+size};
  decoder.advance(size);

  tvAsVariant(this) = unserialize_from_buffer(
    str.data(),
    str.size(),
    VariableUnserializer::Type::Internal
  );
  tvAsVariant(this).setEvalScalar();
}

//////////////////////////////////////////////////////////////////////

}
