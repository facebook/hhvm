<?php
$data = "openssl_seal() test";
$cipher = 'AES-128-CBC';
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$priv_key = "file://" . dirname(__FILE__) . "/private.key";

openssl_seal($data, $sealed, $ekeys, array($pub_key, $pub_key), $cipher);
openssl_seal($data, $sealed, $ekeys, array($pub_key, $pub_key), 'sparkles', $iv);
openssl_seal($data, $sealed, $ekeys, array($pub_key, $pub_key), $cipher, $iv);
openssl_open($sealed, $decrypted, $ekeys[0], $priv_key, $cipher, $iv);
echo $decrypted;
