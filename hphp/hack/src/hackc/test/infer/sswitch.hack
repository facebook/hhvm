// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define C$static._86cinit
// CHECK: define C$static._86cinit($this: *C$static, $constName: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$constName
// CHECK:   n1 = $builtins.hhbc_cmp_same($builtins.hack_string("CONST_REQUIRES_CINIT"), n0)
// CHECK:   jmp b1, b2
// CHECK: #b1:
// CHECK:   prune $builtins.hack_is_true(n1)
// CHECK:   n2 = __sil_lazy_class_initialize(<D>)
// CHECK:   n3 = $builtins.hack_field_get(n2, "STRING")
// CHECK:   n4 = $builtins.hack_new_dict($builtins.hack_string("var"), $builtins.hack_string("id"), $builtins.hack_string("type"), n3)
// CHECK:   ret n4
// CHECK: #b2:
// CHECK:   prune ! $builtins.hack_is_true(n1)
// CHECK:   n5: *HackMixed = load &$constName
// CHECK:   n6 = $builtins.hhbc_concat($builtins.hack_string("Could not find initializer for "), n5, $builtins.hack_string(" in 86cinit"))
// CHECK:   n7 = $builtins.hhbc_fatal(n6)
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
