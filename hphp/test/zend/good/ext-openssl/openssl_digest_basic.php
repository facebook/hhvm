<?php
$data = "openssl_digest() basic test";
$method = "md5";
$method2 = "sha1";

var_dump(openssl_digest($data, $method));
var_dump(openssl_digest($data, $method2));
?>