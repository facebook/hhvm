// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.setop_l
// CHECK: define $root.setop_l($this: *void, $i: *HackInt, $s: *HackString, $f: *HackFloat) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$i
// CHECK:   n1 = $builtins.hhbc_bit_and(n0, $builtins.hack_int(5))
// CHECK:   store &$i <- n1: *HackMixed
// CHECK:   n2 = $builtins.hhbc_bit_or(n1, $builtins.hack_int(5))
// CHECK:   store &$i <- n2: *HackMixed
// CHECK:   n3 = $builtins.hhbc_modulo(n2, $builtins.hack_int(5))
// CHECK:   store &$i <- n3: *HackMixed
// CHECK:   n4 = $builtins.hhbc_bit_xor(n3, $builtins.hack_int(5))
// CHECK:   store &$i <- n4: *HackMixed
// CHECK:   n5 = $builtins.hhbc_shl(n4, $builtins.hack_int(5))
// CHECK:   store &$i <- n5: *HackMixed
// CHECK:   n6 = $builtins.hhbc_shr(n5, $builtins.hack_int(5))
// CHECK:   store &$i <- n6: *HackMixed
// CHECK:   n7: *HackMixed = load &$s
// CHECK:   n8 = $builtins.hhbc_concat(n7, $builtins.hack_int(5))
// CHECK:   store &$s <- n8: *HackMixed
// CHECK:   n9: *HackMixed = load &$f
// CHECK:   n10 = $builtins.hhbc_div(n9, $builtins.hack_int(5))
// CHECK:   store &$f <- n10: *HackMixed
// CHECK:   n11 = $builtins.hhbc_sub(n10, $builtins.hack_int(5))
// CHECK:   store &$f <- n11: *HackMixed
// CHECK:   n12 = $builtins.hhbc_mul(n11, $builtins.hack_int(5))
// CHECK:   store &$f <- n12: *HackMixed
// CHECK:   n13 = $builtins.hhbc_add(n12, $builtins.hack_int(5))
// CHECK:   store &$f <- n13: *HackMixed
// CHECK:   n14 = $builtins.hhbc_pow(n13, $builtins.hack_int(5))
// CHECK:   store &$f <- n14: *HackMixed
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
