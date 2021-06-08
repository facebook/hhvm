<?hh <<__EntryPoint>> function main(): void {
$a = sodium_crypto_core_ristretto255_scalar_random();
$b = sodium_crypto_core_ristretto255_scalar_random();
var_dump($a != $b);
$one = "\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";
$ainv = sodium_crypto_core_ristretto255_scalar_invert($a);
var_dump(sodium_crypto_core_ristretto255_scalar_mul($a, $ainv) == $one);
var_dump(sodium_crypto_core_ristretto255_scalar_invert($one) == $one);
$negone = sodium_crypto_core_ristretto255_scalar_negate($one);
$nega = sodium_crypto_core_ristretto255_scalar_negate($a);
var_dump($nega == sodium_crypto_core_ristretto255_scalar_mul($a, $negone));
$acomp = sodium_crypto_core_ristretto255_scalar_complement($a);
var_dump(sodium_crypto_core_ristretto255_scalar_add($a, $acomp) == $one);
$asubb = sodium_crypto_core_ristretto255_scalar_sub($a, $b);
var_dump(sodium_crypto_core_ristretto255_scalar_add($b, $asubb) == $a);
$bsuba = sodium_crypto_core_ristretto255_scalar_sub($b, $a);
var_dump(sodium_crypto_core_ristretto255_scalar_negate($asubb) == $bsuba);
var_dump(sodium_crypto_core_ristretto255_scalar_add($b, $nega) == $bsuba);
$invalid = "I am a string of invalid length";
$valid = "I am a string with valid length.";
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_invert($invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_negate($invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_complement($invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_mul($invalid, $valid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_mul($valid, $invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_add($invalid, $valid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_add($valid, $invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_sub($invalid, $valid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $subkey = sodium_crypto_core_ristretto255_scalar_sub($valid, $invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
}
