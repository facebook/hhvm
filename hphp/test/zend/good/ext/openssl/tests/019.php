<?php
$data = "Testing openssl_verify()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

openssl_sign($data, $sign, $privkey);
var_dump(openssl_verify($data, $sign, $pubkey));
var_dump(openssl_verify($data, $sign, $privkey));
var_dump(openssl_verify($data, $sign, $wrong));
var_dump(openssl_verify($data, $wrong, $pubkey));
var_dump(openssl_verify($wrong, $sign, $pubkey));
?>