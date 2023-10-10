<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_keygen_aes256gcm() :mixed{
$algo = 'aead_aes256gcm';

printf('%s: ', $algo);
$base = 'sodium_crypto_'.$algo;
$fun = $base.'_keygen';
$const = strtoupper($base.'_KEYBYTES');

$key = $fun();
printf("%s %d %d\n", gettype($key), strlen($key), constant($const));
}
