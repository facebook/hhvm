// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define $root.no_locals
// CHECK: define $root.no_locals($this: *void, $a: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   ret null
// CHECK: }
function no_locals(int $a) : void {
}

// TEST-CHECK-BAL: define $root.only_locals
// CHECK: define $root.only_locals($this: *void) : *void {
// CHECK: local $a: *void, $b: *void
// CHECK: #b0:
// CHECK:   store &$a <- $builtins.hack_int(1): *HackMixed
// CHECK:   store &$b <- $builtins.hack_int(2): *HackMixed
// CHECK:   ret null
// CHECK: }
function only_locals() : void {
  $a = 1;
  $b = 2;
}

// TEST-CHECK-BAL: define $root.params_and_locals
// CHECK: define $root.params_and_locals($this: *void, $a: *HackInt) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK:   store &$b <- $builtins.hack_int(1): *HackMixed
// CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
// CHECK:   ret null
// CHECK: }
function params_and_locals(int $a) : void {
  $b = 1;
  $c = 2;
}
