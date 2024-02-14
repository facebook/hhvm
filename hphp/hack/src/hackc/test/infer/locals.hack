// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

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

// TEST-CHECK-BAL: define $root.locals_for_iter
// CHECK: define $root.locals_for_iter($this: *void, $d: *HackDict) : *void {
// CHECK: local $k: *void, $v: *void, iter0: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK:   n1: *HackMixed = load &$d
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3: *HackMixed = load &$d
// CHECK:   n4 = $builtins.hhbc_iter_init(&iter0, &$k, &$v, n3)
// CHECK:   jmp b1, b6
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n4)
// CHECK:   jmp b2
// CHECK: #b2:
// CHECK:   n5: *HackMixed = load &iter0
// CHECK:   n6 = $builtins.hhbc_iter_next(n5, &$k, &$v)
// CHECK:   jmp b4, b5
// CHECK:   .handlers b3
// CHECK: #b3(n7: *HackMixed):
// CHECK:   n8: *HackMixed = load &iter0
// CHECK:   n9 = $builtins.hhbc_iter_free(n8)
// CHECK:   throw n7
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK:   jmp b7
// CHECK: #b5:
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK:   jmp b2
// CHECK: #b6:
// CHECK:   prune ! $builtins.hack_is_true(n4)
// CHECK:   jmp b7
// CHECK: #b7:
// CHECK:   ret null
// CHECK: }
function locals_for_iter(dict<int, int> $d) : void {
  foreach ($d as $k => $v) {
    // do nothing so we make sure that $k and $v declare locals on their own.
  }
}
