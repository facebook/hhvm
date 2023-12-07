// RUN: %hackc compile-infer --fail-fast -vHack.Lang.AllowUnstableFeatures=1 %s | FileCheck %s

<<file:__EnableUnstableFeatures('expression_trees')>>

// TEST-CHECK-BAL: define $root.basic1
// CHECK: define $root.basic1($this: *void, $b: *A) : *HackMixed {
// CHECK: local $0splice0: *void, $0: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$b
// CHECK:   n1: *HackMixed = load &$0splice0
// CHECK:   n2 = __sil_allocate(<Closure$basic1232>)
// CHECK:   n3 = Closure$basic1232.__construct(n2, null, n0, n1)
// CHECK:   store &$0 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &$0
// CHECK:   n5 = n4.?.__invoke()
// CHECK:   store &$0 <- null: *HackMixed
// CHECK:   ret n5
// CHECK: }

// TEST-CHECK-BAL: define Closure$basic1232.__construct
// CHECK: define Closure$basic1232.__construct($this: *Closure$basic1232, this: *HackMixed, b: *HackMixed, _0splice0: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &b
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.b <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &_0splice0
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?._0splice0 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }
function basic1(A $b): mixed {
  return A`${$b}`;
}
