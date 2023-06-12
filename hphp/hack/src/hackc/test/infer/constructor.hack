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
// CHECK:   n0 = __sil_lazy_class_initialize(<A>)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n1 = __sil_allocate(<A>)
// CHECK:   n2 = A._86pinit(n1)
// CHECK:   store &$2 <- n1: *HackMixed
// CHECK:   n3 = $builtins.hhbc_class_has_reified_generics(n0)
// CHECK:   jmp b2, b5
// CHECK:   .handlers b7
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5 = $builtins.hhbc_has_reified_parent(n4)
// CHECK:   jmp b3, b4
// CHECK:   .handlers b7
// CHECK: #b3:
// CHECK:   prune ! $builtins.hack_is_true(n5)
// CHECK:   jmp b6
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n5)
// CHECK:   n6: *HackMixed = load &$2
// CHECK:   n7 = $builtins.hhbc_cast_vec($builtins.hhbc_new_col_vector())
// CHECK:   n8 = n6.?._86reifiedinit(n7)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b5:
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK:   n9: *HackMixed = load &$0
// CHECK:   n10 = $builtins.hhbc_class_get_c(n9)
// CHECK:   jmp b6
// CHECK:   .handlers b7
// CHECK: #b6:
// CHECK:   n11: *HackMixed = load &$2
// CHECK:   jmp b8
// CHECK:   .handlers b7
// CHECK: #b7(n12: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n13 = $builtins.hhbc_throw(n12)
// CHECK:   unreachable
// CHECK: #b8:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n14 = n11.?.__construct()
// CHECK:   n15 = $builtins.hhbc_lock_obj(n11)
// CHECK:   n16 = n11.?.a()
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
