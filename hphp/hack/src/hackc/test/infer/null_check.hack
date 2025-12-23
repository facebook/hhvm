// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

namespace NullCheck;

class A {
  public function __construct(public string $prop1) {}
}

// TEST-CHECK-BAL: define $root.NullCheck::f1_nonnull
// CHECK: define $root.NullCheck::f1_nonnull($this: *void, $arg: *NullCheck::A) : .notnull *HackString {
// CHECK: #b0:
// CHECK: // .column 7
// CHECK:   n0: *HackMixed = load &$arg
// CHECK: // .column 7
// CHECK:   n1 = $builtins.hhbc_is_type_null(n0)
// CHECK: // .column 7
// CHECK:   n2 = $builtins.hhbc_not(n1)
// CHECK: // .column 7
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 7
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK: // .column 5
// CHECK:   n3 = $builtins.hhbc_is_type_str($builtins.hack_string("default"))
// CHECK: // .column 5
// CHECK:   n4 = $builtins.hhbc_verify_type_pred($builtins.hack_string("default"), n3)
// CHECK: // .column 5
// CHECK:   ret $builtins.hack_string("default")
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 12
// CHECK:   n5: *HackMixed = load &$arg
// CHECK:   n6: *HackMixed = load n5.?.prop1
// CHECK: // .column 5
// CHECK:   n7 = $builtins.hhbc_is_type_str(n6)
// CHECK: // .column 5
// CHECK:   n8 = $builtins.hhbc_verify_type_pred(n6, n7)
// CHECK: // .column 5
// CHECK:   ret n6
// CHECK: }
function f1_nonnull(?A $arg): string {
  if ($arg is nonnull) {
    return $arg->prop1;
  } else {
    return "default";
  }
}

// TEST-CHECK-BAL: define $root.NullCheck::f2_null
// CHECK: define $root.NullCheck::f2_null($this: *void, $arg: *NullCheck::A) : .notnull *HackString {
// CHECK: #b0:
// CHECK: // .column 7
// CHECK:   n0: *HackMixed = load &$arg
// CHECK: // .column 7
// CHECK:   n1 = $builtins.hhbc_is_type_null(n0)
// CHECK: // .column 7
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 7
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK: // .column 12
// CHECK:   n2: *HackMixed = load &$arg
// CHECK:   n3: *HackMixed = load n2.?.prop1
// CHECK: // .column 5
// CHECK:   n4 = $builtins.hhbc_is_type_str(n3)
// CHECK: // .column 5
// CHECK:   n5 = $builtins.hhbc_verify_type_pred(n3, n4)
// CHECK: // .column 5
// CHECK:   ret n3
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK: // .column 5
// CHECK:   n6 = $builtins.hhbc_is_type_str($builtins.hack_string("default"))
// CHECK: // .column 5
// CHECK:   n7 = $builtins.hhbc_verify_type_pred($builtins.hack_string("default"), n6)
// CHECK: // .column 5
// CHECK:   ret $builtins.hack_string("default")
// CHECK: }
function f2_null(?A $arg): string {
  if ($arg is null) {
    return "default";
  } else {
    return $arg->prop1;
  }
}

// TEST-CHECK-BAL: define $root.NullCheck::f3_as_nonnull
// CHECK: define $root.NullCheck::f3_as_nonnull($this: *void, $arg: *NullCheck::A) : .notnull *HackString {
// CHECK: local $0: *void, $1: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(23))
// CHECK: // .column 11
// CHECK:   n1: *HackMixed = load &$arg
// CHECK: // .column 11
// CHECK:   store &$0 <- n1: *HackMixed
// CHECK: // .column 11
// CHECK:   store &$1 <- n0: *HackMixed
// CHECK: // .column 11
// CHECK:   n2 = $builtins.hhbc_is_type_null(n1)
// CHECK: // .column 11
// CHECK:   n3 = $builtins.hhbc_not(n2)
// CHECK: // .column 11
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK: // .column 11
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK: // .column 11
// CHECK:   n4: *HackMixed = load &$0
// CHECK: // .column 11
// CHECK:   store &$1 <- null: *HackMixed
// CHECK: // .column 11
// CHECK:   n5: *HackMixed = load n4.?.prop1
// CHECK: // .column 3
// CHECK:   n6 = $builtins.hhbc_is_type_str(n5)
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_verify_type_pred(n5, n6)
// CHECK: // .column 3
// CHECK:   ret n5
// CHECK: #b2:
// CHECK: // .column 11
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK: // .column 11
// CHECK:   n8: *HackMixed = load &$0
// CHECK: // .column 11
// CHECK:   n9: *HackMixed = load &$1
// CHECK: // .column 11
// CHECK:   n10 = $builtins.hhbc_throw_as_type_struct_exception(n8, n9)
// CHECK:   unreachable
// CHECK: }
function f3_as_nonnull(?A $arg): string {
  return ($arg as nonnull)->prop1;
}
