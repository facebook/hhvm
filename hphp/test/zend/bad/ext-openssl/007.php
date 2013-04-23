<?php
$fp = fopen(dirname(__FILE__) . "/cert.crt","r");
$a = fread($fp,8192);
fclose($fp); 

$b = "file://" . dirname(__FILE__) . "/cert.crt";
$c = "invalid cert";
$d = openssl_x509_read($a);
$e = array();
$f = array($b);

var_dump($res = openssl_x509_read($a));         // read cert as a string
openssl_x509_free($res);
var_dump($res);
var_dump($res = openssl_x509_read($b));         // read cert as a filename string
openssl_x509_free($res);
var_dump($res);
var_dump($res = openssl_x509_read($c));         // read an invalid cert, fails
openssl_x509_free($res);
var_dump($res);
var_dump($res = openssl_x509_read($d));         // read cert from a resource
openssl_x509_free($res);
var_dump($res);
var_dump($res = openssl_x509_read($e));         // read an array
openssl_x509_free($res);
var_dump($res);
var_dump($res = openssl_x509_read($f));         // read an array with the filename
openssl_x509_free($res);
var_dump($res);
?>