// RUN: %hackc compile-infer %s | FileCheck %s

class A {
  public function a() : void {}
}

// TEST-CHECK-BAL: define $root.f1
// CHECK: define $root.f1($this: *void) : *void {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0: *A$static = load &A$static::static_singleton
// CHECK:   n1 = $builtins.lazy_initialize(n0)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n2: *A$static = load &A$static::static_singleton
// CHECK:   n3 = $builtins.lazy_initialize(n2)
// CHECK:   n4 = n2.HackMixed.__factory()
// CHECK:   store &$2 <- n4: *HackMixed
// CHECK:   n5: *HackMixed = load &$0
// CHECK:   n6 = $builtins.hhbc_class_has_reified_generics(n5)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   n7: *HackMixed = load &$0
// CHECK:   n8 = $builtins.hhbc_has_reified_parent(n7)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n8)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n8)
// CHECK:   n9: *HackMixed = load &$2
// CHECK:   n10 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n11 = n9.HackMixed._86reifiedinit(n10)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   n12: *HackMixed = load &$0
// CHECK:   n13 = $builtins.hhbc_class_get_c(n12)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n14: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n15: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n16 = $builtins.hhbc_throw(n15)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n17 = n14.HackMixed.__construct()
// CHECK:   n18 = $builtins.hhbc_lock_obj(n14)
// CHECK:   n19 = n14.HackMixed.a()
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
