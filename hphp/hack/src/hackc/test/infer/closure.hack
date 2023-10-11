// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define Closure$closure1.__invoke
// CHECK: define Closure$closure1.__invoke($this: *Closure$closure1) : *HackMixed {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.x
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   n2 = $builtins.hhbc_print(n1)
// CHECK:   ret n2
// CHECK: }

// TEST-CHECK-BAL: define Closure$closure1.__construct
// CHECK: define Closure$closure1.__construct($this: *Closure$closure1, x: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &x
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.x <- n0: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define $root.closure1
// CHECK: define $root.closure1($this: *void, $x: *HackString) : *HackMixed {
// CHECK: local $y: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$x
// CHECK:   n1 = __sil_allocate(<Closure$closure1>)
// CHECK:   n2 = Closure$closure1.__construct(n1, n0)
// CHECK:   store &$y <- n1: *HackMixed
// CHECK:   ret n1
// CHECK: }
function closure1(string $x): mixed {
  $y = () ==> print($x);
  return $y;
}
