<?hh
<<__EntryPoint>>
function main(): void {
  $key = str_repeat("k", 32);    // KEYBYTES
  $header = str_repeat("h", 10); // far short of HEADERBYTES
  try {
    sodium_crypto_secretstream_xchacha20poly1305_init_pull($key, $header);
    echo "FAIL: no exception thrown\n";
  } catch (SodiumException $e) {
    echo "OK: caught SodiumException\n";
  }
}
