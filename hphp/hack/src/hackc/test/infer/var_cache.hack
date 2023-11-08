// RUN: %hackc compile-infer --enable-var-cache --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.check1
// CHECK: define $root.check1($this: *void, $a: *HackMixed, $b: *HackMixed, $c: *HackMixed) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2: *HackMixed = load &$c
// CHECK:   n3 = $root.call(null, n0, n1, n2)
// CHECK:   n4 = $root.call(null, n2, n1, n2)
// CHECK:   n5 = $root.call(null, n1, n1, n2)
// CHECK:   n6 = $root.call(null, n0, n1, n0)
// CHECK:   ret null
// CHECK: }
function check1(mixed $a, mixed $b, mixed $c): void {
  call($a,$b,$c);
  call($c,$b,$c);
  call($b,$b,$c);
  call($a,$b,$a);
}

// TEST-CHECK-BAL: define $root.check2
// CHECK: define $root.check2($this: *void, $c: *HackInt) : *void {
// CHECK: local $a: *void
// CHECK: #b0:
// CHECK:   store &$a <- $builtins.hack_int(2): *HackMixed
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1 = $root.call(null, n0)
// CHECK:   store &$a <- $builtins.hack_int(7): *HackMixed
// CHECK:   n2: *HackMixed = load &$a
// CHECK:   n3 = $root.call(null, n2)
// CHECK:   ret null
// CHECK: }
function check2(int $c): void  {
  $a = 2;
  call($a);
  $a = 7;
  call($a);
}

// TEST-CHECK-BAL: define $root.check3
// CHECK: define $root.check3($this: *void) : *void {
// CHECK: local $a: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec()
// CHECK:   store &$a <- n0: *HackMixed
// CHECK:   n1 = $root.call(null, n0)
// CHECK:   n2 = $builtins.hack_int(0)
// CHECK:   n3 = $builtins.hack_int(7)
// CHECK:   n4 = $builtins.hack_array_cow_set(n0, n2, n3)
// CHECK:   store &$a <- n4: *HackMixed
// CHECK:   n5 = $root.call(null, n4)
// CHECK:   ret null
// CHECK: }
function check3(): void {
  $a = vec[];
  call($a);
  $a[0] = 7;
  call($a);
}
