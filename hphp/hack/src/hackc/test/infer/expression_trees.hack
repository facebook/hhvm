// RUN: %hackc compile-infer --fail-fast -vHack.Lang.AllowUnstableFeatures=1 %s | FileCheck %s

<<file:__EnableUnstableFeatures('expression_trees')>>

// TEST-CHECK-BAL: define $root.basic1
// CHECK: define $root.basic1($this: *void, $b: *A) : *HackMixed {
// CHECK: local $0: *void
// CHECK: #b0:
// CHECK: // .column 12
// CHECK:   n0: *HackMixed = load &$b
// CHECK: // .column 12
// CHECK:   n1 = __sil_allocate(<Closure$basic1232>)
// CHECK:   n2 = Closure$basic1232.__construct(n1, null, n0)
// CHECK: // .column 12
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 12
// CHECK:   n3: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n4 = n3.?.__invoke()
// CHECK: // .column 12
// CHECK:   jmp b2
// CHECK:   .handlers b1
// CHECK: #b1(n5: *HackMixed):
// CHECK: // .column 12
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 12
// CHECK:   throw n5
// CHECK: #b2:
// CHECK: // .column 12
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   ret n4
// CHECK: }

// TEST-CHECK-BAL: define Closure$basic1232.__construct
// CHECK: define Closure$basic1232.__construct($this: .notnull *Closure$basic1232, this: *HackMixed, b: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &b
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.b <- n2: *HackMixed
// CHECK:   ret null
// CHECK: }
function basic1(A $b): mixed {
  return A`${$b}`;
}
