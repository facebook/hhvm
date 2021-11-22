<?hh <<__EntryPoint>> function main(): void {
$key = random_bytes(SODIUM_CRYPTO_CORE_HCHACHA20_KEYBYTES);
$in1 = random_bytes(SODIUM_CRYPTO_CORE_HCHACHA20_INPUTBYTES);
$in2 = random_bytes(SODIUM_CRYPTO_CORE_HCHACHA20_INPUTBYTES);
$const = random_bytes(SODIUM_CRYPTO_CORE_HCHACHA20_CONSTBYTES);

try {
    $out = sodium_crypto_core_hchacha20("bad input", $key);
} catch (SodiumException $ex) {
    var_dump(true);
}
try {
    $out = sodium_crypto_core_hchacha20($in1, "bad key");
} catch (SodiumException $ex) {
    var_dump(true);
}
try {
    $out = sodium_crypto_core_hchacha20($in1, $key, "bad const");
} catch (SodiumException $ex) {
    var_dump(true);
}

$subkey1 = sodium_crypto_core_hchacha20($in1, $key);
$subkey2 = sodium_crypto_core_hchacha20($in2, $key);
$subkey3 = sodium_crypto_core_hchacha20($in1, $key, $const);
$subkey4 = sodium_crypto_core_hchacha20($in1, $key);

var_dump($subkey1 !== $subkey2);
var_dump($subkey1 !== $subkey3);
var_dump($subkey2 !== $subkey3);
var_dump($subkey1 === $subkey4);

}
