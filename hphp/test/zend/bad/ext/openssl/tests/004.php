<?php

$a = 1;
var_dump(openssl_csr_new(1,$a));
var_dump(openssl_csr_new(1,$a,1,1));
$a = array();
var_dump(openssl_csr_new(array(), $a, array('config' => __DIR__ . DIRECTORY_SEPARATOR . 'openssl.cnf'), array()));

//this leaks
$a = array(1,2);
$b = array(1,2);
var_dump(openssl_csr_new($a, $b, array('config' => __DIR__ . DIRECTORY_SEPARATOR . 'openssl.cnf')));


echo "Done\n";
?>