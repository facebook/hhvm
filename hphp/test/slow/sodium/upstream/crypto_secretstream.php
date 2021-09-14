<?hh <<__EntryPoint>> function main(): void {
$msg1 = random_bytes(random_int(1, 1000));
$msg2 = random_bytes(random_int(1, 1000));
$ad = random_bytes(random_int(1, 1000));

// key gen
$key = sodium_crypto_secretstream_xchacha20poly1305_keygen();
var_dump(strlen($key) === SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_KEYBYTES);

// push init
$push_state_and_header = sodium_crypto_secretstream_xchacha20poly1305_init_push($key);
var_dump(strlen($push_state_and_header[1]) === SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_HEADERBYTES);

// push (encrypt)
$ciphertext1 = sodium_crypto_secretstream_xchacha20poly1305_push(
  inout $push_state_and_header[0], $msg1, $ad,
  SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_MESSAGE);
var_dump(count($ciphertext1) > 0);
$ciphertext2 = sodium_crypto_secretstream_xchacha20poly1305_push(
  inout $push_state_and_header[0], $msg2, $ad,
  SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_FINAL);
var_dump(count($ciphertext2) > 0);

// pull init
$pull_state = sodium_crypto_secretstream_xchacha20poly1305_init_pull($key, $push_state_and_header[1]);

// pull
$result1 = sodium_crypto_secretstream_xchacha20poly1305_pull(inout $pull_state, $ciphertext1, $ad);
$result2 = sodium_crypto_secretstream_xchacha20poly1305_pull(inout $pull_state, $ciphertext2, $ad);

// check
var_dump($ciphertext1 !== $ciphertext2);
var_dump($result1[0] === $msg1);
var_dump($result2[0] === $msg2);
var_dump($result1[1] === SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_MESSAGE);
var_dump($result2[1] === SODIUM_CRYPTO_SECRETSTREAM_XCHACHA20POLY1305_TAG_FINAL);

// try with incorrect key size
$wrong_key = random_bytes(random_int(1, 1000));
try {
    // Switched order
    $pull_state = sodium_crypto_secretstream_xchacha20poly1305_init_pull($wrong_key, $push_state_and_header[1]);
    var_dump(false);
} catch (SodiumException $ex) {
    var_dump(true);
}
}
