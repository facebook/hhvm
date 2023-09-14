<?hh <<__EntryPoint>> function main(): void {
// Note, ristretto255 scalars are interchangeable Ed25519 scalars.
$a = sodium_crypto_core_ristretto255_scalar_random();
$b = sodium_crypto_core_ristretto255_scalar_random();
var_dump($a != $b);
$zero = "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";
$one = "\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";
$two = "\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";
// L+1, where L is the prime order of the usual subgroup on Ed25519
$group_order_plus_one = hex2bin("eed3f55c1a631258d69cf7a2def9de14000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000");
// G, standard base point on Ed25519, compressed y coordinate form with sign bit
$ed25519_base = "Xfffffffffffffffffffffffffffffff";
// 1*G === G
var_dump(sodium_crypto_scalarmult_ed25519_base($one) === $ed25519_base);
var_dump(sodium_crypto_scalarmult_ed25519_base_noclamp($one) === $ed25519_base);
// L+1 === 1 (mod L)
var_dump(sodium_crypto_core_ed25519_scalar_reduce($group_order_plus_one) === $one);
// Basic arithmetic!
var_dump(sodium_crypto_core_ed25519_scalar_add($one, $one) === $two);
var_dump(sodium_crypto_core_ed25519_scalar_add($one, $zero) === $one);
var_dump(sodium_crypto_core_ed25519_scalar_mul($one, $one) === $one);
var_dump(sodium_crypto_core_ed25519_scalar_mul($one, $two) === $two);
var_dump(sodium_crypto_core_ed25519_scalar_mul($zero, $two) === $zero);
$invalid = "I am a string of invalid length";
$valid = "I am a string with valid length.";
try {
  $_ = sodium_crypto_core_ed25519_scalar_reduce($invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  // non-reduced scalars are twice as long; valid also should not work here.
  $_ = sodium_crypto_core_ed25519_scalar_reduce($valid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $_ = sodium_crypto_core_ed25519_scalar_add($invalid, $valid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $_ = sodium_crypto_core_ed25519_scalar_add($valid, $invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
try {
  $_ = sodium_crypto_core_ed25519_scalar_add($invalid, $invalid);
} catch (SodiumException $ex) {
  var_dump(true);
}
var_dump(sodium_crypto_core_ed25519_is_valid_point($ed25519_base));
var_dump(!sodium_crypto_core_ed25519_is_valid_point($one));
$base_plus_base = sodium_crypto_core_ed25519_add($ed25519_base, $ed25519_base);
$base_times_two = sodium_crypto_scalarmult_ed25519_base_noclamp($two);
var_dump($base_plus_base === $base_times_two);
var_dump(sodium_crypto_core_ed25519_sub($base_plus_base, $ed25519_base) === $ed25519_base);
var_dump(sodium_crypto_scalarmult_ed25519_noclamp($one, $ed25519_base) === $ed25519_base);
var_dump(sodium_crypto_scalarmult_ed25519_noclamp($two, $ed25519_base) === $base_times_two);
try {
  $_ = sodium_crypto_scalarmult_ed25519_noclamp($one, $one);
} catch (SodiumException $ex) {
  var_dump(true);
}
}
