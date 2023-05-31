// RUN: %hackc compile-infer %s | FileCheck %s

namespace NullCheck;

class A {
  public function __construct(public string $prop1) {}
}

// TEST-CHECK-BAL: define $root.NullCheck::f1_nonnull
// CHECK: define $root.NullCheck::f1_nonnull($this: *void, $arg: *NullCheck::A) : *HackString {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(23))
// CHECK:   n1: *HackMixed = load &$arg
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1))
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK:   n3 = $builtins.hhbc_is_type_str($builtins.hack_string("default"))
// CHECK:   n4 = $builtins.hhbc_verify_type_pred($builtins.hack_string("default"), n3)
// CHECK:   ret $builtins.hack_string("default")
// CHECK: #b2:
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK:   n5 = &$arg
// CHECK:   n6 = $builtins.hack_string("prop1")
// CHECK:   n7 = $builtins.hack_dim_field_get(n5, n6)
// CHECK:   n8: *HackMixed = load n7
// CHECK:   n9 = $builtins.hhbc_is_type_str(n8)
// CHECK:   n10 = $builtins.hhbc_verify_type_pred(n8, n9)
// CHECK:   ret n8
// CHECK: }
function f1_nonnull(?A $arg): string {
  if ($arg is nonnull) {
    return $arg->prop1;
  } else {
    return "default";
  }
}

// TEST-CHECK-BAL: define $root.NullCheck::f2_null
// CHECK: define $root.NullCheck::f2_null($this: *void, $arg: *NullCheck::A) : *HackString {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_new_dict($builtins.hack_string("kind"), $builtins.hack_int(28))
// CHECK:   n1: *HackMixed = load &$arg
// CHECK:   n2 = $builtins.hhbc_is_type_struct_c(n1, n0, $builtins.hack_int(1))
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune ! $builtins.hack_is_true(n2)
// CHECK:   n3 = &$arg
// CHECK:   n4 = $builtins.hack_string("prop1")
// CHECK:   n5 = $builtins.hack_dim_field_get(n3, n4)
// CHECK:   n6: *HackMixed = load n5
// CHECK:   n7 = $builtins.hhbc_is_type_str(n6)
// CHECK:   n8 = $builtins.hhbc_verify_type_pred(n6, n7)
// CHECK:   ret n6
// CHECK: #b2:
// CHECK:   prune $builtins.hack_is_true(n2)
// CHECK:   n9 = $builtins.hhbc_is_type_str($builtins.hack_string("default"))
// CHECK:   n10 = $builtins.hhbc_verify_type_pred($builtins.hack_string("default"), n9)
// CHECK:   ret $builtins.hack_string("default")
// CHECK: }
function f2_null(?A $arg): string {
  if ($arg is null) {
    return "default";
  } else {
    return $arg->prop1;
  }
}
