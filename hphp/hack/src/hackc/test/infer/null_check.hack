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
// CHECK:   ret $builtins.hack_string("default")
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK: // .column 12
// CHECK:   n3: *HackMixed = load &$arg
// CHECK:   n4: *HackMixed = load n3.?.prop1
// CHECK: // .column 5
// CHECK:   ret n4
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
// CHECK:   ret n3
// CHECK: #b2:
// CHECK: // .column 7
// CHECK:   prune $builtins.hack_is_true(n1)
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
// CHECK: local $0: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(23))
// CHECK: // .column 11
// CHECK:   n1: *HackMixed = load &$arg
// CHECK: // .column 11
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK: // .column 11
// CHECK:   store &$0 <- n0: *HackMixed
// CHECK: // .column 11
// CHECK:   n2 = $builtins.hhbc_is_type_null(n1)
// CHECK: // .column 11
// CHECK:   n3 = $builtins.hhbc_not(n2)
// CHECK: // .column 11
// CHECK:   jmp b2, b4
// CHECK:   .handlers b5
// CHECK: #b2:
// CHECK: // .column 11
// CHECK:   prune $builtins.hack_is_true(n3)
// CHECK: // .column 11
// CHECK:   jmp b3
// CHECK:   .handlers b5
// CHECK: #b3:
// CHECK: // .column 11
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 11
// CHECK:   n4: *HackMixed = load n1.?.prop1
// CHECK: // .column 3
// CHECK:   ret n4
// CHECK: #b4:
// CHECK: // .column 11
// CHECK:   prune ! $builtins.hack_is_true(n3)
// CHECK: // .column 11
// CHECK:   n5: *HackMixed = load &$0
// CHECK: // .column 11
// CHECK:   n6 = $builtins.hhbc_throw_as_type_struct_exception(n1, n5)
// CHECK:   unreachable
// CHECK:   .handlers b5
// CHECK: #b5(n7: *HackMixed):
// CHECK: // .column 11
// CHECK:   store &$0 <- null: *HackMixed
// CHECK: // .column 11
// CHECK:   throw n7
// CHECK: }
function f3_as_nonnull(?A $arg): string {
  return ($arg as nonnull)->prop1;
}
