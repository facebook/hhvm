<?php
$fp = fopen(dirname(__FILE__) . "/cert.crt","r");
$a = fread($fp,8192);
fclose($fp);

$fp = fopen(dirname(__FILE__) . "/private.key","r");
$b = fread($fp,8192);
fclose($fp);

$cert = "file://" . dirname(__FILE__) . "/cert.crt";
$key = "file://" . dirname(__FILE__) . "/private.key";

var_dump(openssl_x509_check_private_key($cert, $key));
var_dump(openssl_x509_check_private_key("", $key));
var_dump(openssl_x509_check_private_key($cert, ""));
var_dump(openssl_x509_check_private_key("", ""));
var_dump(openssl_x509_check_private_key($a, $b));
?>
