// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.check_foreach
// CHECK: define $root.check_foreach($this: *void, $x: *HackVec) : *void {
// CHECK: local $index: *void, iter0: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(20), $builtins.hack_string("generic_types"), $builtins.hhbc_new_vec($builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(4))))
// CHECK:   n1: *HackMixed = load &$x
// CHECK:   n2 = $builtins.hhbc_verify_param_type_ts(n1, n0)
// CHECK:   n3 = $builtins.hhbc_iter_init(&iter0, null, &$index, n1)
// CHECK:   jmp b1, b6
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK:   jmp b2
// CHECK: #b2:
// CHECK:   n4: *HackMixed = load &$index
// CHECK:   n5 = $builtins.hhbc_print(n4)
// CHECK:   n6: *HackMixed = load &iter0
// CHECK:   n7 = $builtins.hhbc_iter_next(n6, null, &$index)
// CHECK:   jmp b4, b5
// CHECK:   .handlers b3
// CHECK: #b3(n8: *HackMixed):
// CHECK:   n9: *HackMixed = load &iter0
// CHECK:   n10 = $builtins.hhbc_iter_free(n9)
// CHECK:   n11 = $builtins.hhbc_throw(n8)
// CHECK:   unreachable
// CHECK: #b4:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   jmp b7
// CHECK: #b5:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   jmp b2
// CHECK: #b6:
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK:   jmp b7
// CHECK: #b7:
// CHECK:   ret null
// CHECK: }
function check_foreach(vec<string> $x): void {
  foreach ($x as $index) {
    echo $index;
  }
}
