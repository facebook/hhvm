<?php

$a = 1;
$b = array(1);
$c = array(1);
$d = array(1);

var_dump(openssl_seal($a, $b, $c, $d));
var_dump(openssl_seal($a, $a, $a, array()));
var_dump(openssl_seal($c, $c, $c, 1));
var_dump(openssl_seal($b, $b, $b, ""));

echo "Done\n";
?>