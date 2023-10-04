// RUN: %hackc compile-infer %s | FileCheck %s

// TEST-CHECK-BAL: define $root.basic
// CHECK: define $root.basic($this: *void) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &gconst::INF
// CHECK:   n1: *HackMixed = load &gconst::NAN
// CHECK:   n2 = $root.printf(null, $builtins.hack_string("%f\n"), n0)
// CHECK:   n3 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float($builtins.hack_const_neginf()))
// CHECK:   n4 = $root.printf(null, $builtins.hack_string("%f\n"), n1)
// CHECK:   n5 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(3.0))
// CHECK:   n6 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(-3.0))
// CHECK:   n7 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(3.14))
// CHECK:   n8 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(-3.14))
// CHECK:   n9 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(1.0000000000000002))
// CHECK:   n10 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(-1.0000000000000002))
// CHECK:   n11 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(1.7976931348623157e308))
// CHECK:   n12 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(-1.7976931348623157e308))
// CHECK:   n13 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(5e-324))
// CHECK:   n14 = $root.printf(null, $builtins.hack_string("%f\n"), $builtins.hack_float(-5e-324))
// CHECK:   ret null
// CHECK: }
function basic(): void {
  // Some "named" floats.
  printf("%f\n", INF);
  printf("%f\n", -INF);
  printf("%f\n", NAN);
  // No decimals
  printf("%f\n", 3.0);
  printf("%f\n", -3.0);
  // A "typical" float
  printf("%f\n", 3.14);
  printf("%f\n", -3.14);
  // Maximum precision
  printf("%f\n", 1.0000000000000002);
  printf("%f\n", -1.0000000000000002);
  // maxfloat
  printf("%f\n", 1.7976931348623157e+308);
  printf("%f\n", -1.7976931348623157e+308);
  // denormalized
  printf("%f\n", 5e-324);
  printf("%f\n", -5e-324);
}
