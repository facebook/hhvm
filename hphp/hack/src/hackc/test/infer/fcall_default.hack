// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

class C {
  // TEST-CHECK-BAL: define .wrapper C.myfn2
  // CHECK: define .wrapper C.myfn2($this: .notnull *C, $a: .notnull *HackInt) : *void {
  // CHECK: local $b: *void, $c: *void
  // CHECK: #b0:
  // CHECK: // .column 10
  // CHECK:   store &$b <- $builtins.hack_int(1): *HackMixed
  // CHECK: // .column 10
  // CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
  // CHECK: // .column 10
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1: *HackMixed = load &$b
  // CHECK:   n2: *HackMixed = load &$c
  // CHECK:   n3: *HackMixed = load &$this
  // CHECK:   n4 = n3.?.myfn2(n0, n1, n2)
  // CHECK:   ret n4
  // CHECK: }

  // TEST-CHECK-BAL: define .wrapper C.myfn2
  // CHECK: define .wrapper C.myfn2($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackInt) : *void {
  // CHECK: local $c: *void
  // CHECK: #b0:
  // CHECK: // .column 10
  // CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
  // CHECK: // .column 10
  // CHECK:   jmp b1
  // CHECK: #b1:
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK:   n1: *HackMixed = load &$b
  // CHECK:   n2: *HackMixed = load &$c
  // CHECK:   n3: *HackMixed = load &$this
  // CHECK:   n4 = n3.?.myfn2(n0, n1, n2)
  // CHECK:   ret n4
  // CHECK: }

  // TEST-CHECK-BAL: define C.myfn2
  // CHECK: define C.myfn2($this: .notnull *C, $a: .notnull *HackInt, $b: .notnull *HackInt, $c: .notnull *HackInt) : *void {
  // CHECK: #b0:
  // CHECK: // .column 26
  // CHECK:   n0: *HackMixed = load &$a
  // CHECK: // .column 30
  // CHECK:   n1: *HackMixed = load &$b
  // CHECK: // .column 34
  // CHECK:   n2: *HackMixed = load &$c
  // CHECK: // .column 5
  // CHECK:   n3 = $root.printf(null, $builtins.hack_string("%d %d %d\n"), n0, n1, n2)
  // CHECK: // .column 4
  // CHECK:   ret null
  // CHECK: }
  public function myfn2(int $a, int $b = 1, int $c = 2): void {
    printf("%d %d %d\n", $a, $b, $c);
  }
}

// TEST-CHECK-BAL: define .wrapper $root.myfn
// CHECK: define .wrapper $root.myfn($this: *void, $a: .notnull *HackInt) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK: // .column 1
// CHECK:   store &$b <- $builtins.hack_int(1): *HackMixed
// CHECK: // .column 1
// CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
// CHECK: // .column 1
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2: *HackMixed = load &$c
// CHECK:   n3 = $root.myfn(null, n0, n1, n2)
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-BAL: define .wrapper $root.myfn
// CHECK: define .wrapper $root.myfn($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt) : *void {
// CHECK: local $c: *void
// CHECK: #b0:
// CHECK: // .column 1
// CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
// CHECK: // .column 1
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2: *HackMixed = load &$c
// CHECK:   n3 = $root.myfn(null, n0, n1, n2)
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-BAL: define $root.myfn
// CHECK: define $root.myfn($this: *void, $a: .notnull *HackInt, $b: .notnull *HackInt, $c: .notnull *HackInt) : *void {
// CHECK: #b0:
// CHECK: // .column 24
// CHECK:   n0: *HackMixed = load &$a
// CHECK: // .column 28
// CHECK:   n1: *HackMixed = load &$b
// CHECK: // .column 32
// CHECK:   n2: *HackMixed = load &$c
// CHECK: // .column 3
// CHECK:   n3 = $root.printf(null, $builtins.hack_string("%d %d %d\n"), n0, n1, n2)
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function myfn(int $a, int $b = 1, int $c = 2): void {
  printf("%d %d %d\n", $a, $b, $c);
}
