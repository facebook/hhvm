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
// CHECK:   jmp b2
// CHECK:   .handlers b4
// CHECK: #b2:
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK:   n3: *HackMixed = load &$2
// CHECK:   jmp b5
// CHECK:   .handlers b4
// CHECK: #b4(n4: *HackMixed):
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n5 = $builtins.hhbc_throw(n4)
// CHECK:   unreachable
// CHECK: #b5:
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   store &$1 <- null: *HackMixed
// CHECK:   store &$2 <- null: *HackMixed
// CHECK:   n6 = n3.?.__construct()
// CHECK:   n7 = $builtins.hhbc_lock_obj(n3)
// CHECK:   n8 = n3.?.a()
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
