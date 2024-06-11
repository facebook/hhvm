// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

<<file: __EnableUnstableFeatures('case_types')>>

// TEST-CHECK-1: type MyAlias
// CHECK: type MyAlias equals HH::int
type MyAlias = int;

// TEST-CHECK-1: type MyVec
// CHECK: type MyVec equals HH::vec
type MyVec<T> = vec<T>;

// TEST-CHECK-1: type CT3
// CHECK: type CT3 equals HH::string, HH::bool
case type CT3 = string | bool;
