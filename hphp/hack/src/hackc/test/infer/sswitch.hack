// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define C$static._86cinit
// CHECK: define C$static._86cinit($this: *C$static, $constName: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0 = $builtins.hack_string("id")
// CHECK:   n1 = $builtins.hack_string("type")
// CHECK:   n2 = $builtins.hack_string(" in 86cinit")
// CHECK:   n3 = $builtins.hack_string("var")
// CHECK:   n4 = $builtins.hack_string("Could not find initializer for ")
// CHECK:   n5 = $builtins.hack_string("CONST_REQUIRES_CINIT")
// CHECK:   n6: *HackMixed = load &$constName
// CHECK:   n7 = $builtins.hhbc_cmp_same(n5, n6)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n7)
// CHECK:   n8: *HackMixed = load &D$static::STRING
// CHECK:   n9 = $builtins.hack_new_dict(n3, n0, n1, n8)
// CHECK:   ret n9
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n7)
// CHECK:   n10: *HackMixed = load &$constName
// CHECK:   n11 = $builtins.hhbc_concat(n4, n10, n2)
// CHECK:   n12 = $builtins.hhbc_fatal(n11)
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
