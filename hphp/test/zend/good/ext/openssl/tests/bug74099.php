<?hh <<__EntryPoint>> function main(): void {
$aad = random_bytes(32);
$iv = random_bytes(16);
$key = random_bytes(32);

$plaintext = '';
$tag = null;

$ciphertext = openssl_encrypt_with_tag($plaintext, 'aes-256-gcm', $key, \OPENSSL_RAW_DATA, $iv, inout $tag, $aad);
var_dump($ciphertext);
}
