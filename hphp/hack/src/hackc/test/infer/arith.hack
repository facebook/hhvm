// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.setop_l
// CHECK: define $root.setop_l($this: *void, $i: *HackInt, $s: *HackString, $f: *HackFloat) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$i
// CHECK:   n1 = $builtins.hhbc_bit_and(n0, $builtins.hack_int(5))
// CHECK:   store &$i <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$i
// CHECK:   n3 = $builtins.hhbc_bit_or(n2, $builtins.hack_int(5))
// CHECK:   store &$i <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$i
// CHECK:   n5 = $builtins.hhbc_modulo(n4, $builtins.hack_int(5))
// CHECK:   store &$i <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$i
// CHECK:   n7 = $builtins.hhbc_bit_xor(n6, $builtins.hack_int(5))
// CHECK:   store &$i <- n7: *HackMixed
// CHECK:   n8: *HackMixed = load &$i
// CHECK:   n9 = $builtins.hhbc_shl(n8, $builtins.hack_int(5))
// CHECK:   store &$i <- n9: *HackMixed
// CHECK:   n10: *HackMixed = load &$i
// CHECK:   n11 = $builtins.hhbc_shr(n10, $builtins.hack_int(5))
// CHECK:   store &$i <- n11: *HackMixed
// CHECK:   n12: *HackMixed = load &$s
// CHECK:   n13 = $builtins.hhbc_concat(n12, $builtins.hack_int(5))
// CHECK:   store &$s <- n13: *HackMixed
// CHECK:   n14: *HackMixed = load &$f
// CHECK:   n15 = $builtins.hhbc_div(n14, $builtins.hack_int(5))
// CHECK:   store &$f <- n15: *HackMixed
// CHECK:   n16: *HackMixed = load &$f
// CHECK:   n17 = $builtins.hhbc_sub(n16, $builtins.hack_int(5))
// CHECK:   store &$f <- n17: *HackMixed
// CHECK:   n18: *HackMixed = load &$f
// CHECK:   n19 = $builtins.hhbc_mul(n18, $builtins.hack_int(5))
// CHECK:   store &$f <- n19: *HackMixed
// CHECK:   n20: *HackMixed = load &$f
// CHECK:   n21 = $builtins.hhbc_add(n20, $builtins.hack_int(5))
// CHECK:   store &$f <- n21: *HackMixed
// CHECK:   n22: *HackMixed = load &$f
// CHECK:   n23 = $builtins.hhbc_pow(n22, $builtins.hack_int(5))
// CHECK:   store &$f <- n23: *HackMixed
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
