// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: "type A "
// CHECK: type A extends HH::BuiltinEnum = .kind="class" {
// CHECK: }

// TEST-CHECK-BAL: define A$static.$init_static
// CHECK: define A$static.$init_static() : void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.alloc_words(0)
// CHECK:   store &A$static::static_singleton <- n0: *A$static
// CHECK:   store &A$static::V <- $builtins.hack_int(1): *HackMixed
// CHECK:   ret 0
// CHECK: }

enum A: int {
  V = 1;
}

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: local base: *HackMixed
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("A::V = ")
// CHECK:   n1 = $builtins.hack_string("B::V = ")
// CHECK:   n2 = $builtins.hack_string("\n")
// CHECK:   n3 = $builtins.hhbc_print(n0)
// CHECK:   n4: *HackMixed = load &A$static::V
// CHECK:   n5 = $builtins.hhbc_print(n4)
// CHECK:   n6 = $builtins.hhbc_print(n2)
// CHECK:   n7 = $builtins.hhbc_print(n1)
// CHECK:   n8: *HackMixed = load &B$static::V
// CHECK:   n9 = $builtins.hhbc_print(n8)
// CHECK:   n10 = $builtins.hhbc_print(n2)
// CHECK:   ret null
// CHECK: }
function main(): void {
  echo "A::V = ", A::V, "\n";
  echo "B::V = ", B::V, "\n";
}

// TEST-CHECK-1: "global A$static::V "
// CHECK: global A$static::V : *HackInt

// TEST-CHECK-1: "global B$static::V "
// CHECK: global B$static::V : *HackMixed
