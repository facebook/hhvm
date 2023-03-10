// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define C$static._86cinit
// CHECK: define C$static._86cinit($this: *C$static, $constName: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$constName
// CHECK:   n1 = $builtins.hhbc_cmp_same($builtins.hack_string("CONST_REQUIRES_CINIT"), n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2: *HackMixed = load &const::D$static::STRING
// CHECK:   n3 = $builtins.hack_new_dict($builtins.hack_string("var"), $builtins.hack_string("id"), $builtins.hack_string("type"), n2)
// CHECK:   ret n3
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n4: *HackMixed = load &$constName
// CHECK:   n5 = $builtins.hhbc_concat($builtins.hack_string("Could not find initializer for "), n4, $builtins.hack_string(" in 86cinit"))
// CHECK:   n6 = $builtins.hhbc_fatal(n5)
// CHECK:   unreachable
// CHECK: }

class C {
  // Because this constant has a foreign reference in it (`D::STRING`), it
  // forces the class to get an '86cinit' method to initialize it.
  const mixed CONST_REQUIRES_CINIT = shape(
      'var' => 'id',
      'type' => D::STRING,
    );
}
