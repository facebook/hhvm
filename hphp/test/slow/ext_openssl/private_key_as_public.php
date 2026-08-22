<?hh

// A private key passed where a public key is expected must be accepted rather
// than rejected with "Don't know how to get public key from this private key".
// OpenSSL 3 hands back keys that retain their private components even from
// public-only accessors such as openssl_csr_get_public_key(), so rejecting
// them would break every caller that verifies with a key it also signs with.
<<__EntryPoint>>
function main(): void {
  $privkey = openssl_pkey_new();
  if ($privkey === false) {
    echo "openssl_pkey_new failed\n";
    return;
  }

  $data = "some secret messages";
  $signature = null;
  var_dump(openssl_sign($data, inout $signature, $privkey, 'sha256'));
  var_dump(openssl_verify($data, $signature, $privkey, 'sha256'));
}
