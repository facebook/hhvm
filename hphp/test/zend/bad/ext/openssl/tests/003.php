<?php

function myErrorHandler($errno, $errstr, $errfile, $errline) {
var_dump($errstr);
} 
set_error_handler("myErrorHandler"); 

$a = 1; 
$b = 1; 
$c = new stdclass; 
$d = new stdclass; 

var_dump(openssl_pkcs7_decrypt($a, $b, $c, $d));
var_dump($c);

var_dump(openssl_pkcs7_decrypt($b, $b, $b, $b));
var_dump(openssl_pkcs7_decrypt($a, $b, "", ""));
var_dump(openssl_pkcs7_decrypt($a, $b, true, false));
var_dump(openssl_pkcs7_decrypt($a, $b, 0, 0));

echo "Done\n";
?>