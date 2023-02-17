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
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("A::V = "))
// CHECK:   n1: *HackMixed = load &A$static::V
// CHECK:   n2 = $builtins.hhbc_print(n1)
// CHECK:   n3 = $builtins.hhbc_print($builtins.hack_string("\n"))
// CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("B::V = "))
// CHECK:   n5: *HackMixed = load &B$static::V
// CHECK:   n6 = $builtins.hhbc_print(n5)
// CHECK:   n7 = $builtins.hhbc_print($builtins.hack_string("\n"))
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
