<?php
$m1 = random_bytes(100);
$m2 = random_bytes(100);

$key = sodium_crypto_secretstream_xchacha20poly1305_keygen();
list($state, $header) = sodium_crypto_secretstream_xchacha20poly1305_init_push($key);

$c1 = sodium_crypto_secretstream_xchacha20poly1305_push($state, $m1);
$c2 = sodium_crypto_secretstream_xchacha20poly1305_push($state, $m2);

print "plaintext in === ciphertext\n";
var_dump($m1 === $c1);
var_dump($m1 === $c2);

$state = sodium_crypto_secretstream_xchacha20poly1305_init_pull($header, $key);
list($p1, $_) = sodium_crypto_secretstream_xchacha20poly1305_pull($state, $c1);
list($p2, $_) = sodium_crypto_secretstream_xchacha20poly1305_pull($state, $c2);

print "plaintext in === plaintext out\n";
var_dump($m1 === $p1);
var_dump($m2 === $p2);

print "out of order\n";
$state = sodium_crypto_secretstream_xchacha20poly1305_init_pull($header, $key);
var_dump(sodium_crypto_secretstream_xchacha20poly1305_pull($state, $c2));

print "double read\n";
$state = sodium_crypto_secretstream_xchacha20poly1305_init_pull($header, $key);
sodium_crypto_secretstream_xchacha20poly1305_pull($state, $c1);
var_dump(sodium_crypto_secretstream_xchacha20poly1305_pull($state, $c1));

