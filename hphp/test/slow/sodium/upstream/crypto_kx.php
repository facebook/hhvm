<?php
$client_seed = sodium_hex2bin('0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef');
$client_keypair = sodium_crypto_kx_seed_keypair($client_seed);
$server_seed = sodium_hex2bin('f123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde0');
$server_keypair = sodium_crypto_kx_seed_keypair($server_seed);

var_dump(sodium_bin2hex($client_keypair));
var_dump(sodium_bin2hex($server_keypair));

$client_session_keys =
  sodium_crypto_kx_client_session_keys($client_keypair,
    sodium_crypto_kx_publickey($server_keypair));

$server_session_keys =
  sodium_crypto_kx_server_session_keys($server_keypair,
    sodium_crypto_kx_publickey($client_keypair));

var_dump(sodium_bin2hex($client_session_keys[0]));
var_dump(sodium_bin2hex($server_session_keys[1]));
var_dump(sodium_bin2hex($client_session_keys[1]));
var_dump(sodium_bin2hex($server_session_keys[0]));
?>
