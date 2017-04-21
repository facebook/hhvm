<?php
$data = "openssl_encrypt() tests";
$method = "AES-128-CBC";
$password = "openssl";
$iv = str_repeat("\0", openssl_cipher_iv_length($method));
$wrong = "wrong";
$object = new stdclass;
$arr = array(1);

// wrong paramters tests
var_dump(openssl_encrypt($data, $wrong, $password));
var_dump(openssl_encrypt($object, $method, $password));
var_dump(openssl_encrypt($data, $object, $password));
var_dump(openssl_encrypt($data, $method, $object));
var_dump(openssl_encrypt($arr, $method, $object));
var_dump(openssl_encrypt($data, $arr, $object));
var_dump(openssl_encrypt($data, $method, $arr));

// invalid using of an authentication tag
var_dump(openssl_encrypt($data, $method, $password, 0, $iv, $wrong));
?>
