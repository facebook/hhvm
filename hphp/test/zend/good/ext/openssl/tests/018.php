<?php
$data = "Testing openssl_sign()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";

var_dump(openssl_sign($data, $sign, $privkey));                 // no output
var_dump(openssl_sign($data, $sign, $wrong));
var_dump(openssl_sign(array(), $sign, $privkey));
?>