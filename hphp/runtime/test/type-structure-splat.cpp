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

#include <array>
#include <optional>
#include <vector>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers.h"

// Unit tests for the shape-splat merge (TypeStructure::mergeResolvedShapeSplat),
// the runtime counterpart of the representable subset of OCaml
// Typing_shape_normalize. The case table here is kept aligned with the OCaml
// unit test (hphp/hack/test/unit/typing/shapeSplatMergeTest.ml) so both engines
// are verified against the same semantics where runtime type structures
// preserve the operand. Intersections are currently emitted as mixed.

namespace HPHP {

namespace {

using Kind = TypeStructure::Kind;

const StaticString
  s_fields("fields"),
  s_optional_shape_field("optional_shape_field"),
  s_nullable("nullable"),
  s_allows_unknown_fields("allows_unknown_fields"),
  s_variadic_type("variadic_type"),
  s_splat_elem_types("splat_elem_types"),
  s_union_types("union_types"),
  s_soft("soft"),
  s_alias("alias"),
  s_typevars("typevars"),
  s_id("id"),
  s_x("x"),
  s_y("y"),
  s_z("z"),
  s_a("a"),
  s_b("b"),
  s_c("c");

Array tsKind(Kind k) {
  Array a = Array::CreateDict();
  TypeStructure::setKind(a, k);
  return a;
}

// A shape field value of the given kind, optionally marked optional.
Array field(Kind k, bool optional = false) {
  auto f = tsKind(k);
  if (optional) f.set(s_optional_shape_field, make_tv<KindOfBoolean>(true));
  return f;
}

Array nullableNothing() {
  auto f = field(Kind::T_nothing);
  f.set(s_nullable, make_tv<KindOfBoolean>(true));
  return f;
}

Array optionalField(Array f) {
  f.set(s_optional_shape_field, make_tv<KindOfBoolean>(true));
  return f;
}

Array closedShape(const Array& fields) {
  auto s = tsKind(Kind::T_shape);
  s.set(s_fields, Variant(fields));
  return s;
}

Array openShape(const Array& fields) {
  auto s = closedShape(fields);
  s.set(s_allows_unknown_fields, make_tv<KindOfBoolean>(true));
  return s;
}

Array typedOpenShape(const Array& fields, const Array& unknown) {
  auto s = openShape(fields);
  s.set(s_variadic_type, Variant(unknown));
  return s;
}

Array merge(const req::vector<Array>& elems, bool& invalid) {
  return TypeStructure::mergeResolvedShapeSplat(elems, invalid);
}

Array fieldOf(const Array& shape, const StaticString& name) {
  return shape[s_fields].asCArrRef()[name].asCArrRef();
}

// A T_union type structure over the given member type structures.
Array unionTS(const std::vector<Array>& members) {
  auto u = tsKind(Kind::T_union);
  VecInit v(members.size());
  for (auto const& m : members) v.append(Variant(m));
  u.set(s_union_types, Variant(v.toArray()));
  return u;
}

std::vector<Array> unionMembers(const Array& u) {
  std::vector<Array> out;
  auto const arr = u[s_union_types].asCArrRef();
  auto const n = arr.size();
  for (auto i = 0; i < n; i++) {
    auto const v = arr->nvGetVal(i);
    out.push_back(tvAsCVarRef(&v).toArray());
  }
  return out;
}

// An unresolved shape splat over the given (resolved or unresolved) element
// type structures: {kind: T_shape, splat_elem_types: [...]}.
Array shapeSplat(const std::vector<Array>& elems) {
  auto s = tsKind(Kind::T_shape);
  VecInit v(elems.size());
  for (auto const& e : elems) v.append(Variant(e));
  s.set(s_splat_elem_types, Variant(v.toArray()));
  return s;
}

Array resolveTS(const Array& ts) {
  req::vector<Array> tsList;
  bool persistent = false;
  return TypeStructure::resolve(ts, nullptr, nullptr, tsList, persistent);
}

// -----------------------------------------------------------------------------
// Property tests
//
// The tests below check the algebraic laws over an enumerated space of shapes,
// extending the OCaml property-test space with runtime-specific nullable and
// typed-open type structures. Flat properties use mergeResolvedShapeSplat;
// nested-splat properties use resolve, where flattening actually occurs.
// -----------------------------------------------------------------------------

const std::array<const StaticString*, 3> kFieldNames{{&s_a, &s_b, &s_c}};

bool hasField(const Array& shape, const StaticString& name) {
  if (!shape.exists(s_fields)) return false;
  return shape[s_fields].asCArrRef().exists(name);
}

bool isOptional(const Array& fieldVal) {
  return fieldVal.exists(s_optional_shape_field);
}

bool typeStructureSame(const Array& a, const Array& b) {
  return a.get()->same(b.get());
}

std::vector<Array> fieldTypes() {
  return {
    field(Kind::T_int),
    field(Kind::T_bool),
    field(Kind::T_float),
    nullableNothing(),
  };
}

// Each field is absent, or required/optional of one of fieldTypes(): 9 states.
std::vector<std::optional<Array>> fieldStates() {
  std::vector<std::optional<Array>> out{std::nullopt};
  for (auto const& ty : fieldTypes()) {
    out.emplace_back(ty);
    out.emplace_back(optionalField(ty));
  }
  return out;
}

// Closed, untyped-open, and two explicit unknown-field bounds.
std::vector<Array> unknownTypes() {
  return {
    field(Kind::T_nothing),
    field(Kind::T_mixed),
    field(Kind::T_string),
    nullableNothing(),
  };
}

bool isBottom(const Array& ts) {
  return TypeStructure::kind(ts) == Kind::T_nothing &&
         !ts.exists(s_nullable);
}

Array shapeWithUnknown(const Array& fields, const Array& unknown) {
  if (isBottom(unknown)) return closedShape(fields);
  if (TypeStructure::kind(unknown) == Kind::T_mixed) {
    return openShape(fields);
  }
  return typedOpenShape(fields, unknown);
}

Array buildShape(
    const std::optional<Array>& fa,
    const std::optional<Array>& fb,
    const std::optional<Array>& fc,
    const Array& unknown) {
  auto fields = Array::CreateDict();
  auto add = [&](const StaticString& n, const std::optional<Array>& f) {
    if (f) fields.set(n, Variant(*f));
  };
  add(s_a, fa);
  add(s_b, fb);
  add(s_c, fc);
  return shapeWithUnknown(fields, unknown);
}

// The full space: 3 fields x 9 states x 4 unknown bounds = 2,916 shapes. Used
// by the linear (O(n)) properties.
std::vector<Array> allShapes() {
  std::vector<Array> out;
  auto const states = fieldStates();
  auto const unknowns = unknownTypes();
  for (auto const& fa : states) {
    for (auto const& fb : states) {
      for (auto const& fc : states) {
        for (auto const& unknown : unknowns) {
          out.push_back(buildShape(fa, fb, fc, unknown));
        }
      }
    }
  }
  return out;
}

// A two-field projection (9^2 x 4 = 324 shapes) keeps pairwise (O(n^2))
// properties tractable. Rightmost-wins and key-union are per-field /
// per-operand-set laws, so two fields already cover every relevant combination.
std::vector<Array> pairShapes() {
  std::vector<Array> out;
  auto const states = fieldStates();
  auto const unknowns = unknownTypes();
  for (auto const& fa : states) {
    for (auto const& fb : states) {
      for (auto const& unknown : unknowns) {
        out.push_back(buildShape(fa, fb, std::nullopt, unknown));
      }
    }
  }
  return out;
}

} // namespace

// shape('x' => int) + shape('y' => string)  -->  both required, closed.
TEST(TypeStructureSplat, DisjointClosed) {
  auto left = closedShape(make_dict_array("x", Variant(field(Kind::T_int))));
  auto right = closedShape(make_dict_array("y", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);

  EXPECT_FALSE(invalid);
  EXPECT_EQ(TypeStructure::kind(merged), Kind::T_shape);
  EXPECT_FALSE(merged.exists(s_allows_unknown_fields));
  EXPECT_EQ(TypeStructure::kind(fieldOf(merged, s_x)), Kind::T_int);
  EXPECT_EQ(TypeStructure::kind(fieldOf(merged, s_y)), Kind::T_string);
  EXPECT_FALSE(fieldOf(merged, s_x).exists(s_optional_shape_field));
  EXPECT_FALSE(fieldOf(merged, s_y).exists(s_optional_shape_field));
}

// Rightmost wins: shape('a' => int) + shape('a' => string)  -->  a: string.
TEST(TypeStructureSplat, RightmostWins) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto right = closedShape(make_dict_array("a", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);
  EXPECT_EQ(TypeStructure::kind(fieldOf(merged, s_a)), Kind::T_string);
}

// An optional left field is discarded when the right field is required.
TEST(TypeStructureSplat, OptionalRequiredRightmostWins) {
  auto left =
    closedShape(make_dict_array("a", Variant(field(Kind::T_int, true))));
  auto right =
    closedShape(make_dict_array("a", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);

  EXPECT_FALSE(invalid);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_string);
}

// Required left, optional right  -->  required union.
TEST(TypeStructureSplat, RequiredOptionalUnion) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto right =
    closedShape(make_dict_array("a", Variant(field(Kind::T_string, true))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field)); // required
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_union);
  auto const members = unionMembers(a);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(TypeStructure::kind(members[0]), Kind::T_int);
  EXPECT_EQ(TypeStructure::kind(members[1]), Kind::T_string);
}

// Optional left, optional right  -->  optional union.
TEST(TypeStructureSplat, OptionalOptionalUnion) {
  auto left =
    closedShape(make_dict_array("a", Variant(field(Kind::T_int, true))));
  auto right =
    closedShape(make_dict_array("a", Variant(field(Kind::T_string, true))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);

  EXPECT_FALSE(invalid);
  auto a = fieldOf(merged, s_a);
  EXPECT_TRUE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_union);
  auto const members = unionMembers(a);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(TypeStructure::kind(members[0]), Kind::T_int);
  EXPECT_EQ(TypeStructure::kind(members[1]), Kind::T_string);
}

// `?nothing` denotes null, not bottom. It must remain in a field union.
TEST(TypeStructureSplat, NullableNothingIsNotUnionIdentity) {
  auto left =
    closedShape(make_dict_array("a", Variant(nullableNothing())));
  auto right =
    closedShape(make_dict_array("a", Variant(field(Kind::T_int, true))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);

  EXPECT_FALSE(invalid);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_union);
  auto const members = unionMembers(a);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(TypeStructure::kind(members[0]), Kind::T_nothing);
  EXPECT_TRUE(members[0].exists(s_nullable));
  EXPECT_EQ(TypeStructure::kind(members[1]), Kind::T_int);
}

// Explicit unknown-field bounds participate in field and row unions.
TEST(TypeStructureSplat, TypedOpenBounds) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto right = typedOpenShape(Array::CreateDict(), field(Kind::T_string));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, right}, invalid);

  EXPECT_FALSE(invalid);
  EXPECT_TRUE(merged.exists(s_allows_unknown_fields));
  EXPECT_EQ(
    TypeStructure::kind(merged[s_variadic_type].asCArrRef()), Kind::T_string);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_union);
  auto const members = unionMembers(a);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(TypeStructure::kind(members[0]), Kind::T_int);
  EXPECT_EQ(TypeStructure::kind(members[1]), Kind::T_string);
}

// A nullable-nothing unknown bound permits unknown fields whose value is null;
// it is not the closed-row sentinel.
TEST(TypeStructureSplat, NullableNothingUnknownKeepsRowOpen) {
  auto open = typedOpenShape(Array::CreateDict(), nullableNothing());
  bool invalid = false;
  auto merged = merge(req::vector<Array>{open}, invalid);

  EXPECT_FALSE(invalid);
  EXPECT_TRUE(merged.exists(s_allows_unknown_fields));
  auto const unknown = merged[s_variadic_type].asCArrRef();
  EXPECT_EQ(TypeStructure::kind(unknown), Kind::T_nothing);
  EXPECT_TRUE(unknown.exists(s_nullable));
}

// A field present only on the left, with an open shape on the right, is unioned
// with the right's unknown (mixed) and absorbed to mixed.
TEST(TypeStructureSplat, OpenRightAbsorbsToMixed) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto open = openShape(Array::CreateDict());
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, open}, invalid);
  EXPECT_TRUE(merged.exists(s_allows_unknown_fields));
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_mixed);
}

// Spreading `dynamic` on the right opens the row with `dynamic` as the
// unknown-field bound and unions a field to its left with `dynamic`:
// shape('a' => int) + dynamic  -->  a: (int | dynamic), open, variadic dynamic.
TEST(TypeStructureSplat, DynamicOnRightUnionsField) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{left, tsKind(Kind::T_dynamic)}, invalid);

  EXPECT_FALSE(invalid);
  EXPECT_TRUE(merged.exists(s_allows_unknown_fields));
  EXPECT_EQ(
    TypeStructure::kind(merged[s_variadic_type].asCArrRef()), Kind::T_dynamic);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field)); // still required
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_union); // int | dynamic
}

// Spreading `dynamic` on the left leaves a rightmost concrete field unchanged
// but still opens the row with `dynamic`:
// dynamic + shape('a' => int)  -->  a: int (required), open, variadic dynamic.
TEST(TypeStructureSplat, DynamicOnLeftFieldWins) {
  auto right = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged =
    merge(req::vector<Array>{tsKind(Kind::T_dynamic), right}, invalid);

  EXPECT_FALSE(invalid);
  EXPECT_TRUE(merged.exists(s_allows_unknown_fields));
  EXPECT_EQ(
    TypeStructure::kind(merged[s_variadic_type].asCArrRef()), Kind::T_dynamic);
  auto a = fieldOf(merged, s_a);
  EXPECT_FALSE(a.exists(s_optional_shape_field));
  EXPECT_EQ(TypeStructure::kind(a), Kind::T_int);
}

TEST(TypeStructureSplat, NullableDynamicIsInvalidResidual) {
  auto nullableDynamic = tsKind(Kind::T_dynamic);
  nullableDynamic.set(s_nullable, make_tv<KindOfBoolean>(true));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{nullableDynamic}, invalid);

  EXPECT_TRUE(invalid);
  EXPECT_TRUE(typeStructureSame(merged, shapeSplat({nullableDynamic})));
}

TEST(TypeStructureSplat, BottomStillMarksNullableDynamicInvalid) {
  auto nullableDynamic = tsKind(Kind::T_dynamic);
  nullableDynamic.set(s_nullable, make_tv<KindOfBoolean>(true));
  bool invalid = false;
  auto merged = merge(
    req::vector<Array>{tsKind(Kind::T_nothing), nullableDynamic}, invalid);

  EXPECT_TRUE(invalid);
  EXPECT_EQ(TypeStructure::kind(merged), Kind::T_nothing);
}

// Spreading nothing yields the bottom row, collapsing the whole shape.
TEST(TypeStructureSplat, BottomAbsorbs) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged =
    merge(req::vector<Array>{left, tsKind(Kind::T_nothing)}, invalid);
  EXPECT_EQ(TypeStructure::kind(merged), Kind::T_nothing);
}

// An empty intersection denotes mixed and hackc emits intersections as mixed.
// Mixed is not a shape, so preserve it as an invalid residual.
TEST(TypeStructureSplat, MixedOperandIsInvalidResidual) {
  bool invalid = false;
  auto merged = merge(req::vector<Array>{tsKind(Kind::T_mixed)}, invalid);

  EXPECT_TRUE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_shape);
  ASSERT_TRUE(merged.exists(s_splat_elem_types));
  auto const elems = merged[s_splat_elem_types].asCArrRef();
  ASSERT_EQ(elems.size(), 1u);
  EXPECT_EQ(
    TypeStructure::kind(elems[0].asCArrRef()), Kind::T_mixed
  );
}

// Bottom determines the result but must not suppress invalid operands on
// either side of it.
TEST(TypeStructureSplat, BottomStillMarksInvalidOperands) {
  auto const nothing = tsKind(Kind::T_nothing);
  auto const mixed = tsKind(Kind::T_mixed);
  for (auto const bottomFirst : {false, true}) {
    bool invalid = false;
    auto const merged = bottomFirst
      ? merge(req::vector<Array>{nothing, mixed}, invalid)
      : merge(req::vector<Array>{mixed, nothing}, invalid);

    EXPECT_TRUE(invalid) << "bottomFirst=" << bottomFirst;
    EXPECT_EQ(TypeStructure::kind(merged), Kind::T_nothing);
  }
}

TEST(TypeStructureSplat, ResolverRejectsInvalidOperandErasedByBottom) {
  auto const splat = shapeSplat({
    tsKind(Kind::T_mixed),
    tsKind(Kind::T_nothing),
  });
  req::vector<Array> tsList;

  EXPECT_THROW(
    resolveAndVerifyTypeStructure<true>(
      splat, nullptr, nullptr, tsList, false
    ),
    FatalErrorException
  );
}

TEST(TypeStructureSplat, BottomStillMarksMalformedShapeInvalid) {
  auto const malformedShape = tsKind(Kind::T_shape);
  bool invalid = false;
  auto const merged = merge(
    req::vector<Array>{tsKind(Kind::T_nothing), malformedShape}, invalid);

  EXPECT_TRUE(invalid);
  EXPECT_EQ(TypeStructure::kind(merged), Kind::T_nothing);
}

// A residual operand (type variable) cannot be flattened: invalid + keeps the
// element list.
TEST(TypeStructureSplat, ResidualTypevar) {
  auto left = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged =
    merge(req::vector<Array>{left, tsKind(Kind::T_typevar)}, invalid);
  EXPECT_TRUE(invalid);
  EXPECT_TRUE(merged.exists(s_splat_elem_types));
}

TEST(TypeStructureSplat, ResidualTrailingEmptyIsIdentity) {
  auto residual = tsKind(Kind::T_typevar);
  auto empty = closedShape(Array::CreateDict());
  bool invalid = false;
  auto merged = merge(req::vector<Array>{residual, empty}, invalid);

  EXPECT_TRUE(invalid);
  auto const expected = shapeSplat({residual});
  EXPECT_TRUE(typeStructureSame(merged, expected));
}

// A nested splat which remains residual must be spliced into the outer list;
// treating the residual T_shape as an ordinary shape would drop it because it
// has splat_elem_types rather than fields.
TEST(TypeStructureSplat, NestedResidualSplatIsFlattened) {
  auto nested = shapeSplat({
    closedShape(make_dict_array("y", Variant(field(Kind::T_bool)))),
    tsKind(Kind::T_typevar),
  });
  auto outer = shapeSplat({
    closedShape(make_dict_array("x", Variant(field(Kind::T_int)))),
    nested,
    closedShape(make_dict_array("z", Variant(field(Kind::T_string)))),
  });
  req::vector<Array> tsList;
  bool persistent = false;
  auto resolved =
    TypeStructure::resolve(outer, nullptr, nullptr, tsList, persistent);

  ASSERT_EQ(TypeStructure::kind(resolved), Kind::T_shape);
  ASSERT_TRUE(resolved.exists(s_splat_elem_types));
  auto const elems = resolved[s_splat_elem_types].asCArrRef();
  ASSERT_EQ(elems.size(), 3u);
  auto const prefix = elems[0].asCArrRef();
  EXPECT_TRUE(hasField(prefix, s_x));
  EXPECT_TRUE(hasField(prefix, s_y));
  EXPECT_EQ(TypeStructure::kind(elems[1].asCArrRef()), Kind::T_typevar);
  auto const suffix = elems[2].asCArrRef();
  EXPECT_TRUE(hasField(suffix, s_z));
}

// Property: any field required in the right operand appears required with the
// right operand's exact kind in the merge.
TEST(TypeStructureSplat, PropRightmostWins) {
  auto const shapes = pairShapes();
  for (auto const& left : shapes) {
    for (auto const& right : shapes) {
      bool invalid = false;
      auto const merged = merge(req::vector<Array>{left, right}, invalid);
      ASSERT_FALSE(invalid);
      ASSERT_EQ(TypeStructure::kind(merged), Kind::T_shape);
      for (auto const* np : kFieldNames) {
        auto const& n = *np;
        if (!hasField(right, n)) continue;
        auto const rf = fieldOf(right, n);
        if (isOptional(rf)) continue;
        ASSERT_TRUE(hasField(merged, n));
        auto const mf = fieldOf(merged, n);
        EXPECT_FALSE(isOptional(mf));
        EXPECT_TRUE(typeStructureSame(mf, rf));
      }
    }
  }
}

// Property: when the right field is optional, both field types are preserved
// exactly and the left field alone determines the resulting optionality.
TEST(TypeStructureSplat, PropOptionalFieldMergePreservesBothTypes) {
  auto types = fieldTypes();
  types.push_back(unionTS({field(Kind::T_int), field(Kind::T_bool)}));
  for (auto const& leftType : types) {
    for (auto const& rightType : types) {
      for (auto const leftOptional : {false, true}) {
        auto const leftField =
          leftOptional ? optionalField(leftType) : leftType;
        auto const rightField = optionalField(rightType);
        auto const left =
          closedShape(make_dict_array("a", Variant(leftField)));
        auto const right =
          closedShape(make_dict_array("a", Variant(rightField)));
        bool invalid = false;
        auto const merged = merge(req::vector<Array>{left, right}, invalid);

        ASSERT_FALSE(invalid);
        auto expected = unionTS({leftType, rightType});
        if (leftOptional) expected = optionalField(expected);
        EXPECT_TRUE(typeStructureSame(fieldOf(merged, s_a), expected));
      }
    }
  }
}

// Property: the merged field set is exactly the union of the operands' fields.
TEST(TypeStructureSplat, PropKeysUnion) {
  auto const shapes = pairShapes();
  for (auto const& left : shapes) {
    for (auto const& right : shapes) {
      bool invalid = false;
      auto const merged = merge(req::vector<Array>{left, right}, invalid);
      for (auto const* np : kFieldNames) {
        auto const& n = *np;
        EXPECT_EQ(hasField(merged, n), hasField(left, n) || hasField(right, n));
      }
    }
  }
}

// Property: unknown-field bounds use the same bottom, top, and exact-union
// rules as ordinary field types.
TEST(TypeStructureSplat, PropUnknownBoundsMergeExactly) {
  auto const unknowns = unknownTypes();
  for (auto const& leftUnknown : unknowns) {
    for (auto const& rightUnknown : unknowns) {
      auto const left = shapeWithUnknown(Array::CreateDict(), leftUnknown);
      auto const right = shapeWithUnknown(Array::CreateDict(), rightUnknown);
      bool invalid = false;
      auto const merged = merge(req::vector<Array>{left, right}, invalid);

      ASSERT_FALSE(invalid);
      auto expectedUnknown = unionTS({leftUnknown, rightUnknown});
      if (isBottom(leftUnknown)) expectedUnknown = rightUnknown;
      if (isBottom(rightUnknown)) expectedUnknown = leftUnknown;
      if (TypeStructure::kind(leftUnknown) == Kind::T_mixed ||
          TypeStructure::kind(rightUnknown) == Kind::T_mixed) {
        expectedUnknown = field(Kind::T_mixed);
      }
      auto const expected =
        shapeWithUnknown(Array::CreateDict(), expectedUnknown);
      EXPECT_TRUE(typeStructureSame(merged, expected));
    }
  }
}

// Property: a nothing operand collapses the merge to nothing, in either
// position.
TEST(TypeStructureSplat, PropBottomAbsorbs) {
  auto const nothing = tsKind(Kind::T_nothing);
  for (auto const& s : allShapes()) {
    bool invalid = false;
    auto const m1 = merge(req::vector<Array>{s, nothing}, invalid);
    EXPECT_EQ(TypeStructure::kind(m1), Kind::T_nothing);
    auto const m2 = merge(req::vector<Array>{nothing, s}, invalid);
    EXPECT_EQ(TypeStructure::kind(m2), Kind::T_nothing);
  }
}

// Property: the empty closed shape is a unit for merge, in either position.
TEST(TypeStructureSplat, PropEmptyIdentity) {
  auto const empty = closedShape(Array::CreateDict());
  for (auto const& s : allShapes()) {
    bool invalid = false;
    auto const m1 = merge(req::vector<Array>{s, empty}, invalid);
    auto const m2 = merge(req::vector<Array>{empty, s}, invalid);
    EXPECT_TRUE(typeStructureSame(m1, s));
    EXPECT_TRUE(typeStructureSame(m2, s));
  }
}

// Property: resolving a nested splat is identical to splicing its elements
// into the enclosing splat, including when the nested result stays residual.
TEST(TypeStructureSplat, PropNestedSplatMatchesFlattenedElements) {
  auto const prefix =
    closedShape(make_dict_array("x", Variant(field(Kind::T_int))));
  auto const suffix =
    closedShape(make_dict_array("z", Variant(field(Kind::T_string))));
  auto const residual = tsKind(Kind::T_typevar);
  for (auto const& middle : pairShapes()) {
    auto const nested = shapeSplat({middle, residual});
    auto const nestedResult =
      resolveTS(shapeSplat({prefix, nested, suffix}));
    auto const flatResult =
      resolveTS(shapeSplat({prefix, middle, residual, suffix}));
    EXPECT_TRUE(typeStructureSame(nestedResult, flatResult));
  }
}

// -----------------------------------------------------------------------------
// Union distribution: shape(...A, ...(m1|...|mk), ...B) distributes to
// shape(...A, ...m1, ...B) | ... | shape(...A, ...mk, ...B). The Hack
// typechecker performs the same distribution during normalization.
// -----------------------------------------------------------------------------

// shape('a' => int, ...(shape('b' => int) | shape('c' => string)))
//   -->  shape('a' => int, 'b' => int) | shape('a' => int, 'c' => string).
TEST(TypeStructureSplat, UnionDistributes) {
  auto prefix = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto m1 = closedShape(make_dict_array("b", Variant(field(Kind::T_int))));
  auto m2 = closedShape(make_dict_array("c", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{prefix, unionTS({m1, m2})}, invalid);

  EXPECT_FALSE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_union);
  auto members = unionMembers(merged);
  ASSERT_EQ(members.size(), 2u);
  // Each branch carries the prefix field plus its own member field.
  EXPECT_TRUE(hasField(members[0], s_a));
  EXPECT_TRUE(hasField(members[0], s_b));
  EXPECT_TRUE(hasField(members[1], s_a));
  EXPECT_TRUE(hasField(members[1], s_c));
}

TEST(TypeStructureSplat, UnionAfterResidualDistributesAcrossWholeRow) {
  auto residual = tsKind(Kind::T_typevar);
  auto m1 = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto m2 = closedShape(make_dict_array("b", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged =
    merge(req::vector<Array>{residual, unionTS({m1, m2})}, invalid);

  EXPECT_TRUE(invalid);
  auto const expected = unionTS({
    shapeSplat({residual, m1}),
    shapeSplat({residual, m2}),
  });
  EXPECT_TRUE(typeStructureSame(merged, expected));
}

// A union to the LEFT is distributed with the rightmost-wins suffix applied to
// each branch: shape(...(shape('a'=>int) | shape('a'=>bool)), 'a' => string)
//   -->  shape('a'=>string) | shape('a'=>string).
TEST(TypeStructureSplat, UnionSuffixMergedIntoEachBranch) {
  auto m1 = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto m2 = closedShape(make_dict_array("a", Variant(field(Kind::T_bool))));
  auto suffix = closedShape(make_dict_array("a", Variant(field(Kind::T_string))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{unionTS({m1, m2}), suffix}, invalid);

  EXPECT_FALSE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_union);
  for (auto const& mem : unionMembers(merged)) {
    EXPECT_EQ(TypeStructure::kind(fieldOf(mem, s_a)), Kind::T_string);
  }
}

// A single-member union yields the shape directly (no union wrapper).
TEST(TypeStructureSplat, UnionSingleMember) {
  auto m1 = closedShape(make_dict_array("b", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{unionTS({m1})}, invalid);
  EXPECT_FALSE(invalid);
  EXPECT_EQ(TypeStructure::kind(merged), Kind::T_shape);
  EXPECT_TRUE(hasField(merged, s_b));
}

// A union member that collapses to bottom is dropped from the union:
// shape('a'=>int, ...(shape('b'=>int) | nothing))  -->  shape('a'=>int,'b'=>int).
TEST(TypeStructureSplat, UnionDropsBottomBranch) {
  auto prefix = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto m1 = closedShape(make_dict_array("b", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged =
    merge(req::vector<Array>{prefix, unionTS({m1, tsKind(Kind::T_nothing)})},
          invalid);
  EXPECT_FALSE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_shape);
  EXPECT_TRUE(hasField(merged, s_a));
  EXPECT_TRUE(hasField(merged, s_b));
}

// A union member that is itself a union is flattened into a single union:
// ...(shape('a'=>int) | (shape('b'=>int) | shape('c'=>int)))  -->  3 members.
TEST(TypeStructureSplat, NestedUnionFlattened) {
  auto a = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto b = closedShape(make_dict_array("b", Variant(field(Kind::T_int))));
  auto c = closedShape(make_dict_array("c", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged = merge(req::vector<Array>{unionTS({a, unionTS({b, c})})}, invalid);
  EXPECT_FALSE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_union);
  EXPECT_EQ(unionMembers(merged).size(), 3u);
}

// Two union operands distribute to their Cartesian product in source order.
TEST(TypeStructureSplat, MultipleUnionsDistribute) {
  auto a = closedShape(make_dict_array("a", Variant(field(Kind::T_int))));
  auto b = closedShape(make_dict_array("b", Variant(field(Kind::T_int))));
  auto y = closedShape(make_dict_array("y", Variant(field(Kind::T_int))));
  auto z = closedShape(make_dict_array("z", Variant(field(Kind::T_int))));
  bool invalid = false;
  auto merged = merge(
    req::vector<Array>{unionTS({a, b}), unionTS({y, z})}, invalid);

  EXPECT_FALSE(invalid);
  ASSERT_EQ(TypeStructure::kind(merged), Kind::T_union);
  auto const expected = unionTS({
    closedShape(make_dict_array(
      "a", Variant(field(Kind::T_int)),
      "y", Variant(field(Kind::T_int))
    )),
    closedShape(make_dict_array(
      "a", Variant(field(Kind::T_int)),
      "z", Variant(field(Kind::T_int))
    )),
    closedShape(make_dict_array(
      "b", Variant(field(Kind::T_int)),
      "y", Variant(field(Kind::T_int))
    )),
    closedShape(make_dict_array(
      "b", Variant(field(Kind::T_int)),
      "z", Variant(field(Kind::T_int))
    )),
  });
  EXPECT_TRUE(merged.get()->same(expected.get()));
}

// -----------------------------------------------------------------------------
// Reified operand re-resolution (defensive; the path is gated off from valid
// Hack). A reified splat operand whose argument is itself a shape splat arrives
// unresolved (spliced verbatim from tsList); resolveTSImpl must re-resolve it so
// the nested splat is merged rather than silently dropped. Exercised through the
// public tsList-taking resolve overload.
// -----------------------------------------------------------------------------

// type structure `shape(...T, 'z' => bool)` with reified `T = shape(...S1, ...S2)`
// where S1 = shape('x'=>int), S2 = shape('y'=>string). The reified argument is a
// splat shape; the result must be the fully merged shape('x'=>int,'y'=>string,
// 'z'=>bool), not just {'z'=>bool}.
TEST(TypeStructureSplat, ReifiedSplatOperandReResolved) {
  auto s1 = closedShape(make_dict_array("x", Variant(field(Kind::T_int))));
  auto s2 = closedShape(make_dict_array("y", Variant(field(Kind::T_string))));
  // Reified argument tsList[0]: an (unresolved) shape splat over S1, S2.
  req::vector<Array> tsList{shapeSplat({s1, s2})};

  auto reifiedNode = tsKind(Kind::T_reifiedtype);
  reifiedNode.set(s_id, make_tv<KindOfInt64>(0));
  auto outer =
    shapeSplat({reifiedNode,
                closedShape(make_dict_array("z", Variant(field(Kind::T_bool))))});

  bool persistent = false;
  auto resolved =
    TypeStructure::resolve(outer, nullptr, nullptr, tsList, persistent);

  ASSERT_EQ(TypeStructure::kind(resolved), Kind::T_shape);
  EXPECT_TRUE(hasField(resolved, s_x));
  EXPECT_TRUE(hasField(resolved, s_y));
  EXPECT_TRUE(hasField(resolved, s_z));
  EXPECT_EQ(TypeStructure::kind(fieldOf(resolved, s_x)), Kind::T_int);
  EXPECT_EQ(TypeStructure::kind(fieldOf(resolved, s_y)), Kind::T_string);
  EXPECT_EQ(TypeStructure::kind(fieldOf(resolved, s_z)), Kind::T_bool);
}

TEST(TypeStructureSplat, ReifiedSplatPreservesModifiersAndMetadata) {
  const StaticString alias("TReifiedSplat"), typevars("T");
  auto argument = shapeSplat({
    closedShape(make_dict_array("x", Variant(field(Kind::T_int)))),
    closedShape(make_dict_array("y", Variant(field(Kind::T_string)))),
  });
  argument.set(s_nullable, make_tv<KindOfBoolean>(true));
  argument.set(s_soft, make_tv<KindOfBoolean>(true));
  argument.set(s_alias, Variant(alias));
  argument.set(s_typevars, Variant(typevars));

  req::vector<Array> tsList{argument};
  auto reified = tsKind(Kind::T_reifiedtype);
  reified.set(s_id, make_tv<KindOfInt64>(0));

  bool persistent = false;
  auto const resolved =
    TypeStructure::resolve(reified, nullptr, nullptr, tsList, persistent);

  ASSERT_EQ(TypeStructure::kind(resolved), Kind::T_shape);
  EXPECT_TRUE(hasField(resolved, s_x));
  EXPECT_TRUE(hasField(resolved, s_y));
  EXPECT_TRUE(resolved.exists(s_nullable));
  EXPECT_TRUE(resolved.exists(s_soft));
  ASSERT_TRUE(resolved.exists(s_alias));
  ASSERT_TRUE(resolved.exists(s_typevars));
  EXPECT_TRUE(resolved[s_alias].asCStrRef().same(alias));
  EXPECT_TRUE(resolved[s_typevars].asCStrRef().same(typevars));
}

// Displaying an UNRESOLVED splat shape (carrying splat_elem_types, no s_fields)
// must not crash and should render the element list.
TEST(TypeStructureSplat, DisplayUnresolvedSplat) {
  auto splat = tsKind(Kind::T_shape);
  VecInit elems(3);
  elems.append(Variant(tsKind(Kind::T_int)));
  elems.append(
    Variant(closedShape(make_dict_array("a", Variant(field(Kind::T_int))))));
  elems.append(Variant(openShape(Array::CreateDict())));
  splat.set(s_splat_elem_types, Variant(elems.toArray()));

  auto const s = TypeStructure::toString(
    splat, TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(s.isNull());
  EXPECT_EQ("shape(...int, 'a' => int, ...shape(...))", s.toCppString());
}

TEST(TypeStructureSplat, DisplayUnresolvedSplatOpenFieldRun) {
  auto const fields = make_dict_array("a", Variant(field(Kind::T_int)));

  auto const untyped = TypeStructure::toString(
    shapeSplat({openShape(fields)}),
    TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(untyped.isNull());
  EXPECT_EQ("shape(...shape('a' => int, ...))", untyped.toCppString());

  auto const typed = TypeStructure::toString(
    shapeSplat({typedOpenShape(fields, tsKind(Kind::T_bool))}),
    TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(typed.isNull());
  EXPECT_EQ("shape(...shape('a' => int, bool...))", typed.toCppString());
}

TEST(TypeStructureSplat, DisplayUnresolvedSplatMultipleOpenOperands) {
  auto const splat = shapeSplat({
    typedOpenShape(
      make_dict_array("a", Variant(field(Kind::T_int))),
      tsKind(Kind::T_int)
    ),
    typedOpenShape(
      make_dict_array("b", Variant(field(Kind::T_bool))),
      tsKind(Kind::T_string)
    ),
    closedShape(make_dict_array("c", Variant(field(Kind::T_float)))),
  });

  auto const s = TypeStructure::toString(
    splat, TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(s.isNull());
  EXPECT_EQ(
    "shape(...shape('a' => int, int...), "
    "...shape('b' => bool, string...), 'c' => float)",
    s.toCppString());
}

TEST(TypeStructureSplat, DisplayUnresolvedSplatEmptyOperands) {
  auto const splat = shapeSplat({
    closedShape(Array::CreateDict()),
    closedShape(make_dict_array("a", Variant(field(Kind::T_int)))),
    closedShape(Array::CreateDict()),
  });

  auto const s = TypeStructure::toString(
    splat, TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(s.isNull());
  EXPECT_EQ(
    "shape(...shape(), 'a' => int, ...shape())",
    s.toCppString());
}

TEST(TypeStructureSplat, DisplayNestedUnresolvedSplat) {
  auto const nested = shapeSplat({
    tsKind(Kind::T_dynamic),
    closedShape(make_dict_array("a", Variant(field(Kind::T_int)))),
  });
  auto const outer = shapeSplat({
    nested,
    closedShape(make_dict_array("b", Variant(field(Kind::T_bool)))),
  });

  auto const s = TypeStructure::toString(
    outer, TypeStructure::TSDisplayType::TSDisplayTypeUser);
  ASSERT_FALSE(s.isNull());
  EXPECT_EQ(
    "shape(...shape(...dynamic, 'a' => int), 'b' => bool)",
    s.toCppString());
}

} // namespace HPHP
