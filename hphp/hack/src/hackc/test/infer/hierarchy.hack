// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: type A$static
// CHECK: type A$static = .kind="class" .static {
// CHECK: }

// TEST-CHECK-BAL: "type A "
// CHECK: type A = .kind="class" {
// CHECK: }

class A { }

// TEST-CHECK-BAL: type I0$static
// CHECK: type I0$static = .kind="interface" .static {
// CHECK: }

// TEST-CHECK-BAL: "type I0 "
// CHECK: type I0 = .kind="interface" {
// CHECK: }

interface I0 { }

// TEST-CHECK-BAL: type I1$static
// CHECK: type I1$static extends I0$static = .kind="interface" .static {
// CHECK: }

// TEST-CHECK-BAL: "type I1 "
// CHECK: type I1 extends I0 = .kind="interface" {
// CHECK: }

interface I1 extends I0 { }

// TEST-CHECK-BAL: type T$static
// CHECK: type T$static = .kind="trait" .static {
// CHECK: }

// TEST-CHECK-BAL: "type T "
// CHECK: type T = .kind="trait" {
// CHECK: }

trait T { }

// TEST-CHECK-BAL: type B$static
// CHECK: type B$static extends A$static, I1$static, T$static = .kind="class" .static {
// CHECK: }

// TEST-CHECK-BAL: "type B "
// CHECK: type B extends A, I1, T = .kind="class" {
// CHECK: }

class B extends A implements I1 {
  use T;
}
