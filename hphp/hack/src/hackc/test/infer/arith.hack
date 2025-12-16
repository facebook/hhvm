// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.setop_l
// CHECK: define $root.setop_l($this: *void, $i: .notnull *HackInt, $s: .notnull *HackString, $f: .notnull *HackFloat) : *void {
// CHECK: #b0:
// CHECK: // .column 3
// CHECK:   n0: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n1 = $builtins.hhbc_bit_and(n0, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n1: *HackMixed
// CHECK: // .column 3
// CHECK:   n2: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n3 = $builtins.hhbc_bit_or(n2, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n3: *HackMixed
// CHECK: // .column 3
// CHECK:   n4: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n5 = $builtins.hhbc_modulo(n4, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n5: *HackMixed
// CHECK: // .column 3
// CHECK:   n6: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n7 = $builtins.hhbc_bit_xor(n6, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n7: *HackMixed
// CHECK: // .column 3
// CHECK:   n8: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n9 = $builtins.hhbc_shl(n8, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n9: *HackMixed
// CHECK: // .column 3
// CHECK:   n10: *HackMixed = load &$i
// CHECK: // .column 3
// CHECK:   n11 = $builtins.hhbc_shr(n10, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$i <- n11: *HackMixed
// CHECK: // .column 3
// CHECK:   n12: *HackMixed = load &$s
// CHECK: // .column 3
// CHECK:   n13 = $builtins.hhbc_concat(n12, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$s <- n13: *HackMixed
// CHECK: // .column 3
// CHECK:   n14: *HackMixed = load &$f
// CHECK: // .column 3
// CHECK:   n15 = $builtins.hhbc_div(n14, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$f <- n15: *HackMixed
// CHECK: // .column 3
// CHECK:   n16: *HackMixed = load &$f
// CHECK: // .column 3
// CHECK:   n17 = $builtins.hhbc_sub(n16, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$f <- n17: *HackMixed
// CHECK: // .column 3
// CHECK:   n18: *HackMixed = load &$f
// CHECK: // .column 3
// CHECK:   n19 = $builtins.hhbc_mul(n18, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$f <- n19: *HackMixed
// CHECK: // .column 3
// CHECK:   n20: *HackMixed = load &$f
// CHECK: // .column 3
// CHECK:   n21 = $builtins.hhbc_add(n20, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$f <- n21: *HackMixed
// CHECK: // .column 3
// CHECK:   n22: *HackMixed = load &$f
// CHECK: // .column 3
// CHECK:   n23 = $builtins.hhbc_pow(n22, $builtins.hack_int(5))
// CHECK: // .column 3
// CHECK:   store &$f <- n23: *HackMixed
// CHECK: // .column 2
// CHECK:   ret null
// CHECK: }
function setop_l(int $i, string $s, float $f): void {
  $i &= 5;
  $i |= 5;
  $i %= 5;
  $i ^= 5;
  $i <<= 5;
  $i >>= 5;

  $s .= 5;

  $f /= 5;
  $f -= 5;
  $f *= 5;
  $f += 5;
  $f **= 5;
}
