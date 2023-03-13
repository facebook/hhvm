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
// CHECK:   n0: *A$static = load &const::A$static::static_singleton
// CHECK:   n1 = $builtins.lazy_initialize(n0)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n2 = __sil_allocate(<A>)
// CHECK:   n3 = A._86pinit(n2)
// CHECK:   store &$2 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5 = $builtins.hhbc_class_has_reified_generics(n4)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   n6: *HackMixed = load &$0
// CHECK:   n7 = $builtins.hhbc_has_reified_parent(n6)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &$2
// CHECK:   n9 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n10 = n8.HackMixed._86reifiedinit(n9)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   n11: *HackMixed = load &$0
// CHECK:   n12 = $builtins.hhbc_class_get_c(n11)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n13: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n14: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n15 = $builtins.hhbc_throw(n14)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n16 = n13.HackMixed.__construct()
// CHECK:   n17 = $builtins.hhbc_lock_obj(n13)
// CHECK:   n18 = n13.HackMixed.a()
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
