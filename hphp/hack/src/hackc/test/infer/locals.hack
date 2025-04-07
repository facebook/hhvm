// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.no_locals
// CHECK: define $root.no_locals($this: *void, $a: .notnull *HackInt) : *void {
// CHECK: #b0:
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function no_locals(int $a) : void {
}

// TEST-CHECK-BAL: define $root.only_locals
// CHECK: define $root.only_locals($this: *void) : *void {
// CHECK: local $a: *void, $b: *void
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   store &$a <- $builtins.hack_int(1): *HackMixed
// CHECK: // .column 3
// CHECK:   store &$b <- $builtins.hack_int(2): *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function only_locals() : void {
  $a = 1;
  $b = 2;
}

// TEST-CHECK-BAL: define $root.params_and_locals
// CHECK: define $root.params_and_locals($this: *void, $a: .notnull *HackInt) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   store &$b <- $builtins.hack_int(1): *HackMixed
// CHECK: // .column 3
// CHECK:   store &$c <- $builtins.hack_int(2): *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function params_and_locals(int $a) : void {
  $b = 1;
  $c = 2;
}

// TEST-CHECK-BAL: define $root.locals_for_iter
// CHECK: define $root.locals_for_iter($this: *void, $d: .notnull *HackDict) : *void {
// CHECK: local $k: *void, $v: *void, iter0: *void, $0: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(19), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1)), $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(1))))
// CHECK: // .column 1
// CHECK:   n1: *HackMixed = load &$d
// CHECK: // .column 1
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK: // .column 12
// CHECK:   n3: *HackMixed = load &$d
// CHECK: // .column 12
// CHECK:   n4 = $builtins.hhbc_iter_base(n3)
// CHECK: // .column 12
// CHECK:   store &$0 <- n4: *HackMixed
// CHECK: // .column 12
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 12
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n6 = $builtins.hhbc_iter_init(&iter0, n5)
// CHECK: // .column 12
// CHECK:   jmp b2, b7
// CHECK:   .handlers b9
// CHECK: #b2:
// CHECK: // .column 12
// CHECK:   prune $builtins.hack_is_true(n6)
// CHECK: // .column 12
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK: // .column 12
// CHECK:   n7: *HackMixed = load &iter0
// CHECK: // .column 12
// CHECK:   n8: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n9 = $builtins.hhbc_iter_get_value(n7, n8)
// CHECK: // .column 12
// CHECK:   store &$v <- n9: *HackMixed
// CHECK: // .column 12
// CHECK:   n10: *HackMixed = load &iter0
// CHECK: // .column 12
// CHECK:   n11: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n12 = $builtins.hhbc_iter_get_key(n10, n11)
// CHECK: // .column 12
// CHECK:   store &$k <- n12: *HackMixed
// CHECK: // .column 3
// CHECK:   n13: *HackMixed = load &$0
// CHECK: // .column 3
// CHECK:   n14: *HackMixed = load &iter0
// CHECK: // .column 3
// CHECK:   n15 = $builtins.hhbc_iter_next(n14, n13)
// CHECK: // .column 3
// CHECK:   jmp b5, b6
// CHECK:   .handlers b4
// CHECK: #b4(n16: *HackMixed):
// CHECK: // .column 3
// CHECK:   n17: *HackMixed = load &iter0
// CHECK: // .column 3
// CHECK:   n18 = $builtins.hhbc_iter_free(n17)
// CHECK: // .column 3
// CHECK:   throw n16
// CHECK:   .handlers b9
// CHECK: #b5:
// CHECK: // .column 3
// CHECK:   prune $builtins.hack_is_true(n15)
// CHECK: // .column 3
// CHECK:   jmp b8
// CHECK: #b6:
// CHECK: // .column 3
// CHECK:   prune ! $builtins.hack_is_true(n15)
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK: #b7:
// CHECK: // .column 12
// CHECK:   prune ! $builtins.hack_is_true(n6)
// CHECK: // .column 12
// CHECK:   jmp b8
// CHECK: #b8:
// CHECK: // .column 3
// CHECK:   jmp b10
// CHECK:   .handlers b9
// CHECK: #b9(n19: *HackMixed):
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   throw n19
// CHECK: #b10:
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function locals_for_iter(dict<int, int> $d) : void {
  foreach ($d as $k => $v) {
    // do nothing so we make sure that $k and $v declare locals on their own.
  }
}
