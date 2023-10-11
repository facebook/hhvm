// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: "type A "
// CHECK: type A extends HH::BuiltinEnum = .kind="class" {
// CHECK: }

// TEST-CHECK-BAL: define A$static._86sinit
// CHECK: define A$static._86sinit($this: *A$static) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *A$static = load &$this
// CHECK:   n1 = HH::BuiltinEnum$static._86sinit(n0)
// CHECK:   n2 = $builtins.hhbc_class_get_c($builtins.hack_string("A"))
// CHECK:   n3 = $builtins.hack_set_static_prop($builtins.hack_string("A"), $builtins.hack_string("V"), $builtins.hack_int(1))
// CHECK:   ret null
// CHECK: }

enum A: int {
  V = 1;
}

// TEST-CHECK-BAL: define $root.main
// CHECK: define $root.main($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_print($builtins.hack_string("A::V = "))
// CHECK:   n1 = __sil_lazy_class_initialize(<A>)
// CHECK:   n2 = $builtins.hack_field_get(n1, "V")
// CHECK:   n3 = $builtins.hhbc_print(n2)
// CHECK:   n4 = $builtins.hhbc_print($builtins.hack_string("\n"))
// CHECK:   n5 = $builtins.hhbc_print($builtins.hack_string("B::V = "))
// CHECK:   n6 = __sil_lazy_class_initialize(<B>)
// CHECK:   n7 = $builtins.hack_field_get(n6, "V")
// CHECK:   n8 = $builtins.hhbc_print(n7)
// CHECK:   n9 = $builtins.hhbc_print($builtins.hack_string("\n"))
// CHECK:   ret null
// CHECK: }
function main(): void {
  echo "A::V = ", A::V, "\n";
  echo "B::V = ", B::V, "\n";
}
