// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.check_foreach
// CHECK: define $root.check_foreach($this: *void, $x: .notnull *HackVec) : *void {
// CHECK: local $index: *void, iter0: *void, $0: *void
// CHECK: #b0:
// CHECK: // .column 12
// CHECK:   n0: *HackMixed = load &$x
// CHECK: // .column 12
// CHECK:   n1 = $builtins.hhbc_iter_base(n0)
// CHECK: // .column 12
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 12
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 12
// CHECK:   n2: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n3 = $builtins.hhbc_iter_init(&iter0, n2)
// CHECK: // .column 12
// CHECK:   jmp b2, b7
// CHECK:   .handlers b9
// CHECK: #b2:
// CHECK: // .column 12
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK: // .column 12
// CHECK:   jmp b3
// CHECK: #b3:
// CHECK: // .column 12
// CHECK:   n4: *HackMixed = load &iter0
// CHECK: // .column 12
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 12
// CHECK:   n6 = $builtins.hhbc_iter_get_value(n4, n5)
// CHECK: // .column 12
// CHECK:   store &$index <- n6: *HackMixed
// CHECK: // .column 10
// CHECK:   n7: *HackMixed = load &$index
// CHECK: // .column 5
// CHECK:   n8 = $builtins.hhbc_print(n7)
// CHECK: // .column 3
// CHECK:   n9: *HackMixed = load &$0
// CHECK: // .column 3
// CHECK:   n10: *HackMixed = load &iter0
// CHECK: // .column 3
// CHECK:   n11 = $builtins.hhbc_iter_next(n10, n9)
// CHECK: // .column 3
// CHECK:   jmp b5, b6
// CHECK:   .handlers b4
// CHECK: #b4(n12: *HackMixed):
// CHECK: // .column 3
// CHECK:   n13: *HackMixed = load &iter0
// CHECK: // .column 3
// CHECK:   n14 = $builtins.hhbc_iter_free(n13)
// CHECK: // .column 3
// CHECK:   throw n12
// CHECK:   .handlers b9
// CHECK: #b5:
// CHECK: // .column 3
// CHECK:   prune $builtins.hack_is_true(n11)
// CHECK: // .column 3
// CHECK:   jmp b8
// CHECK: #b6:
// CHECK: // .column 3
// CHECK:   prune ! $builtins.hack_is_true(n11)
// CHECK: // .column 3
// CHECK:   jmp b3
// CHECK: #b7:
// CHECK: // .column 12
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK: // .column 12
// CHECK:   jmp b8
// CHECK: #b8:
// CHECK: // .column 3
// CHECK:   jmp b10
// CHECK:   .handlers b9
// CHECK: #b9(n15: *HackMixed):
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   throw n15
// CHECK: #b10:
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 3
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function check_foreach(vec<string> $x): void {
  foreach ($x as $index) {
    echo $index;
  }
}
