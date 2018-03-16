<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

$algo = 'aead_aes256gcm';

printf('%s: ', $algo);
$base = 'sodium_crypto_'.$algo;
$fun = $base.'_keygen';
$const = strtoupper($base.'_KEYBYTES');

$key = $fun();
printf("%s %d %d\n", gettype($key), strlen($key), constant($const));
