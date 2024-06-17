// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

<<file: __EnableUnstableFeatures('case_types')>>

// TEST-CHECK-1: type MyAlias
// CHECK: type MyAlias equals HackInt
type MyAlias = int;

// TEST-CHECK-1: type MyVec
// CHECK: type MyVec equals HackVec
type MyVec<T> = vec<T>;

// TEST-CHECK-1: type CT3
// CHECK: type CT3 equals HackString, HackBool
case type CT3 = string | bool;

// TEST-CHECK-1: type AShapeAlias
// CHECK: type AShapeAlias equals HackArray
type AShapeAlias = shape('success' => bool, 'count' => int);

// TEST-CHECK-1: type ADictAlias
// CHECK: type ADictAlias equals HackDict
type ADictAlias = dict<string, mixed>;

// TEST-CHECK-1: type AFunctionAlias
// CHECK: type AFunctionAlias equals HackMixed
type AFunctionAlias = (function(int): int);
