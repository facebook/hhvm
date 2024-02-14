// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class A {
  public function a() : void {}
}

// TEST-CHECK-BAL: define $root.f1
// CHECK: define $root.f1($this: *void) : *void {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0 = __sil_lazy_class_initialize(<A>)
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK:   n1 = __sil_allocate(<A>)
// CHECK:   n2 = A._86pinit(n1)
// CHECK:   store &$2 <- n1: *HackMixed
// CHECK:   n3: *HackMixed = load &$0
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5: *HackMixed = load &$2
// CHECK:   jmp b2
// CHECK:   .handlers b1
// CHECK: #b1(n6: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   throw n6
// CHECK: #b2:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n7 = n5.?.__construct()
// CHECK:   n8 = $builtins.hhbc_lock_obj(n5)
// CHECK:   n9 = n5.?.a()
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
