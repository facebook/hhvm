// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class A {
  public function a() : void {}
}

// TEST-CHECK-BAL: define $root.f1
// CHECK: define $root.f1($this: *void) : *void {
// CHECK: local $0: *void, $1: *void, $2: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec()
// CHECK: // .column 4
// CHECK:   n1 = __sil_lazy_class_initialize(<A>)
// CHECK: // .column 4
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 4
// CHECK:   n2 = __sil_allocate(<A>)
// CHECK:   n3 = A._86pinit(n2)
// CHECK: // .column 4
// CHECK:   store &$2 <- n2: *HackMixed
// CHECK: // .column 4
// CHECK:   store &$1 <- n0: *HackMixed
// CHECK: // .column 4
// CHECK:   n4: *HackMixed = load &$2
// CHECK: // .column 4
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 4
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   n6: *HackMixed = load &$2
// CHECK: // .column 4
// CHECK:   jmp b2
// CHECK:   .handlers b1
// CHECK: #b1(n7: *HackMixed):
// CHECK: // .column 4
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   store &$2 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   throw n7
// CHECK: #b2:
// CHECK: // .column 4
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   store &$2 <- null: *HackMixed
// CHECK: // .column 4
// CHECK:   n8 = n6.?.__construct()
// CHECK: // .column 4
// CHECK:   n9 = $builtins.hhbc_lock_obj(n6)
// CHECK: // .column 3
// CHECK:   n10 = n6.?.a()
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function f1() : void {
  (new A())->a();
}
