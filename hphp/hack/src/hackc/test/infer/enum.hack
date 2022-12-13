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
// CHECK:   n1 = $builtins.hack_string("\n")
// CHECK:   n2 = $builtins.hhbc_print(n0)
// CHECK:   n3: *HackMixed = load &A$static::V
// CHECK:   n4 = $builtins.hhbc_print(n3)
// CHECK:   n5 = $builtins.hhbc_print(n1)
// CHECK:   ret null
// CHECK: }
function main(): void {
  echo "A::V = ", A::V, "\n";
}

// TEST-CHECK-1: "global A$static::V "
// CHECK: global A$static::V : *HackInt
