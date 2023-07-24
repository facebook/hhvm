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

#include <folly/portability/GTest.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/bespoke/type-structure.h"

namespace HPHP {

namespace {

bool same_arrays(const ArrayData* a1, const ArrayData* a2) {
  return a1->same(a2);
}

}

using Kind = TypeStructure::Kind;

TEST(BespokeTypeStructure, Methods) {
  auto t = StringData::Make("T");
  auto kind = StringData::Make("kind");
  auto soft = StringData::Make("soft");
  auto alias = StringData::Make("alias");

  auto kindInt = Variant{int8_t(Kind::T_int)};

  {
    Array arr = Array::CreateDict();
    arr.set(Variant{"kind"}, kindInt);
    EXPECT_TRUE(arr->isVanillaDict());

    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    EXPECT_TRUE(ts->typeKind() == TypeStructure::Kind::T_int);
    EXPECT_FALSE(ts->nullable());
    EXPECT_FALSE(ts->soft());
    EXPECT_FALSE(ts->opaque());
    EXPECT_FALSE(ts->optionalShapeField());
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant{"kind"}, kindInt);
    arr.set(Variant{"nullable"}, Variant{true});
    arr.set(Variant{"soft"}, Variant{true});
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    EXPECT_TRUE(ts->nullable());
    EXPECT_TRUE(ts->soft());
    EXPECT_FALSE(ts->opaque());
    EXPECT_FALSE(ts->optionalShapeField());
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant{"kind"}, kindInt);
    arr.set(Variant{"alias"}, Variant{t});
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    EXPECT_TRUE(ts->alias() != nullptr);
    EXPECT_TRUE(ts->alias()->same(t));
  }
  {
    Array arr = Array::CreateDict();
    arr.set(Variant{"kind"}, kindInt);
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    auto tvKind = bespoke::TypeStructure::NvGetStr(ts, kind);
    EXPECT_TRUE(tvKind.m_type == KindOfInt64);
    EXPECT_TRUE(val(tvKind).num == int64_t(TypeStructure::Kind::T_int));

    auto tvSoft = bespoke::TypeStructure::NvGetStr(ts, soft);
    EXPECT_TRUE(tvSoft.m_type == KindOfUninit);

    auto tvAlias = bespoke::TypeStructure::NvGetStr(ts, alias);
    EXPECT_TRUE(tvAlias.m_type == KindOfUninit);
  }
  {
    Array arr = make_dict_array(
      "soft", true,
      "kind", int8_t(Kind::T_shape)
    );
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    auto tv = bespoke::TypeStructure::GetPosKey(ts, 0);
    EXPECT_TRUE(isStringType(tv.m_type));
    EXPECT_TRUE(val(tv).pstr->same(soft));
    auto tvUninit = bespoke::TypeStructure::GetPosKey(ts, ts->size());
    EXPECT_TRUE(tvUninit.m_type == KindOfUninit);
  }
  {
    Array arr = make_dict_array(
      "soft", true,
      "kind", int8_t(Kind::T_shape)
    );
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    auto tv = bespoke::TypeStructure::GetPosVal(ts, 0);
    EXPECT_TRUE(tv.m_type == KindOfBoolean);
    EXPECT_TRUE(val(tv).num == 1);
  }
  {
    Array arr = make_dict_array(
      "kind", int8_t(Kind::T_shape),
      "alias", t
    );
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    auto tvKey = bespoke::TypeStructure::GetPosKey(ts, 1);
    EXPECT_TRUE(isStringType(tvKey.m_type));
    EXPECT_TRUE(val(tvKey).pstr->same(alias));
    auto tvVal = bespoke::TypeStructure::GetPosVal(ts, 1);
    EXPECT_TRUE(isStringType(tvVal.m_type));
  }
  {
    // shape type structure = shape("y" => int)
    Array field = make_dict_array("y", make_dict_array("kind", kindInt));
    Array arr = make_dict_array(
      "kind", int8_t(Kind::T_shape),
      "fields", field
    );
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    EXPECT_TRUE(ts->typeKind() == Kind::T_shape);

    auto const tvFields = bespoke::TypeStructure::NvGetStr(ts, StringData::Make("fields"));
    EXPECT_TRUE(isArrayLikeType(tvFields.m_type));

    auto const tvAllowsUnknownFields =
      bespoke::TypeStructure::NvGetStr(ts, StringData::Make("allows_unknown_fields"));
    EXPECT_TRUE(tvAllowsUnknownFields.m_type == KindOfUninit);

    auto const* vad = ts->escalateWithCapacity(ts->size(), __func__);
    EXPECT_TRUE(vad->isVanilla());
    EXPECT_TRUE(vad->size() == ts->size());

    auto const vadKind = vad->get(Variant{"kind"});
    EXPECT_TRUE(vadKind.m_type == KindOfInt64);
    EXPECT_TRUE(val(vadKind).num == int8_t(Kind::T_shape));

    auto const vadFields = vad->get(Variant{"fields"});
    EXPECT_TRUE(isArrayLikeType(vadFields.m_type));
    EXPECT_TRUE(same_arrays(vadFields.val().parr, field.get()));

    bespoke::TypeStructure* ts2 = ts->copy<false>();
    EXPECT_TRUE(ts2->typeKind() == Kind::T_shape);
  }
  {
    // tuple type structure = (string, int)
    Array elemTypes = make_vec_array(
      make_dict_array("kind", int8_t(Kind::T_string)),
      make_dict_array("kind", kindInt)
    );
    Array arr = make_dict_array(
      "kind", int8_t(Kind::T_tuple),
      "elem_types", elemTypes
    );
    auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
    EXPECT_TRUE(ts->typeKind() == Kind::T_tuple);
    auto const* vad = ts->escalateWithCapacity(ts->size(), __func__);
    EXPECT_TRUE(vad->isVanilla());
    EXPECT_TRUE(vad->size() == ts->size());

    auto const vadKind = vad->get(Variant{"kind"});
    EXPECT_TRUE(vadKind.m_type == KindOfInt64);
    EXPECT_TRUE(val(vadKind).num == int8_t(Kind::T_tuple));

    auto const vadFields = vad->get(Variant{"elem_types"});
    EXPECT_TRUE(isArrayLikeType(vadFields.m_type));
    EXPECT_TRUE(same_arrays(vadFields.val().parr, elemTypes.get()));

    bespoke::TypeStructure* ts2 = ts->copy<false>();
    EXPECT_TRUE(ts2->typeKind() == Kind::T_tuple);
  }

  Array arr = Array::CreateDict();
  arr.set(Variant{"kind"}, kindInt);
  arr.set(Variant{"nullable"}, Variant{true});
  arr.set(Variant{"soft"}, Variant{true});
  arr.set(Variant{"alias"}, Variant{t});
  auto ts = bespoke::TypeStructure::MakeFromVanilla(arr.get());
  auto const* vad = bespoke::TypeStructure::EscalateToVanilla(ts, __func__);
  EXPECT_TRUE(vad->isVanilla());

  auto const vadKind = vad->get(Variant{"kind"});
  EXPECT_TRUE(vadKind.m_type == KindOfInt64);
  EXPECT_TRUE(val(vadKind).num == uint64_t(TypeStructure::Kind::T_int));

  auto const vadAlias = vad->get(Variant{"alias"});
  EXPECT_TRUE(isStringType(vadAlias.m_type));
  EXPECT_TRUE(val(vadAlias).pstr->same(t));

  auto const ts2 = ts->copy<false>();
  EXPECT_TRUE(ts2->typeKind() == TypeStructure::Kind::T_int);
  EXPECT_TRUE(ts2->nullable());
  EXPECT_TRUE(ts2->soft());
  EXPECT_FALSE(ts2->opaque());
  EXPECT_FALSE(ts2->optionalShapeField());
  EXPECT_TRUE(ts2->alias() != nullptr);
  EXPECT_TRUE(ts2->alias()->same(t));
  EXPECT_TRUE(ts2->typevars() == nullptr);
  EXPECT_TRUE(ts2->typevarTypes() == nullptr);
}

}
