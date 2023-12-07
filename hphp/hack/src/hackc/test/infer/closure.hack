// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.closure1
// CHECK: define $root.closure1($this: *void, $x: *HackString) : *HackMixed {
// CHECK: local $y: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$x
// CHECK:   n1 = __sil_allocate(<Closure$closure1>)
// CHECK:   n2 = Closure$closure1.__construct(n1, null, n0)
// CHECK:   store &$y <- n1: *HackMixed
// CHECK:   n3: *HackMixed = load &$y
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-BAL: define Closure$closure1.__construct
// CHECK: define Closure$closure1.__construct($this: *Closure$closure1, this: *HackMixed, x: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &x
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.x <- n2: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define Closure$closure1.__invoke
// CHECK: define Closure$closure1.__invoke($this: *Closure$closure1) : *HackMixed {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.x
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.this
// CHECK:   store &$this <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$x
// CHECK:   n5 = $builtins.hhbc_print(n4)
// CHECK:   ret n5
// CHECK: }

function closure1(string $x): mixed {
  $y = () ==> print($x);
  return $y;
}
