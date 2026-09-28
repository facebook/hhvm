<?hh
// RUN: %hackc compile -v Hack.Lang.AllowUnstableFeatures=true %s | FileCheck %s

<<file:__EnableUnstableFeatures('like_type_hints')>>

class W<reify T> {}

class Vec<reify T> {}

// CHECK-LABEL: p_vec(
// CHECK-NOT: VerifyParamTypeTS
function p_vec<reify T>(vec<T> $x): void {}

// CHECK-LABEL: p_dict(
// CHECK-NOT: VerifyParamTypeTS
function p_dict<reify T>(dict<string, T> $x): void {}

// CHECK-LABEL: p_keyset(
// CHECK-NOT: VerifyParamTypeTS
function p_keyset<reify T as arraykey>(keyset<T> $x): void {}

// CHECK-LABEL: p_varray(
// CHECK-NOT: VerifyParamTypeTS
function p_varray<reify T>(varray<T> $x): void {}

// CHECK-LABEL: p_darray(
// CHECK-NOT: VerifyParamTypeTS
function p_darray<reify T>(darray<string, T> $x): void {}

// CHECK-LABEL: p_vec_or_dict(
// CHECK-NOT: VerifyParamTypeTS
function p_vec_or_dict<reify T>(vec_or_dict<T> $x): void {}

// CHECK-LABEL: p_varray_or_darray(
// CHECK-NOT: VerifyParamTypeTS
function p_varray_or_darray<reify T>(varray_or_darray<T> $x): void {}

// CHECK-LABEL: p_any_array(
// CHECK-NOT: VerifyParamTypeTS
function p_any_array<reify T>(AnyArray<string, T> $x): void {}

// CHECK-LABEL: p_nullable_vec(
// CHECK-NOT: VerifyParamTypeTS
function p_nullable_vec<reify T>(?vec<T> $x): void {}

// CHECK-LABEL: p_like_vec(
// CHECK-NOT: VerifyParamTypeTS
function p_like_vec<reify T>(~vec<T> $x): void {}

// CHECK-LABEL: p_soft_vec(
// CHECK-NOT: VerifyParamTypeTS
function p_soft_vec<reify T>(<<__Soft>> vec<T> $x): void {}

// CHECK-LABEL: r_vec(
// CHECK-NOT: VerifyRetTypeTS
function r_vec<reify T>(mixed $x): vec<T> { return $x; }

// CHECK-LABEL: p_w_vec(
// CHECK: CombineAndResolveTypeStruct 2
// CHECK: VerifyParamTypeTS $x
function p_w_vec<reify T>(W<vec<T>> $x): void {}

// CHECK-LABEL: r_w_vec(
// CHECK: CombineAndResolveTypeStruct 2
// CHECK: VerifyRetTypeTS
function r_w_vec<reify T>(mixed $x): W<vec<T>> { return $x; }

// CHECK-LABEL: p_user_vec(
// CHECK: CombineAndResolveTypeStruct 2
// CHECK: VerifyParamTypeTS $x
function p_user_vec<reify T>(Vec<T> $x): void {}
