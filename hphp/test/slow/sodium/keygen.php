<?hh


<<__EntryPoint>>
function main_keygen() :mixed{
$algos = vec[
  'aead_chacha20poly1305',
  'aead_chacha20poly1305_ietf',
  'aead_xchacha20poly1305_ietf',
  'auth',
  'generichash',
  'kdf',
  'secretbox',
  'shorthash',
  'stream',
];

foreach ($algos as $algo) {
  printf('%s: ', $algo);
  $base = 'sodium_crypto_'.$algo;
  $fun = $base.'_keygen';
  $const = strtoupper($base.'_KEYBYTES');

  $key = $fun();
  printf("%s %d %d\n", gettype($key), strlen($key), constant($const));
}
}
