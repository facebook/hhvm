<?php

var_dump($n = gmp_init(42));
var_dump($s = serialize($n));
var_dump(unserialize($s));

$n = gmp_init(13);
$n->foo = "bar";
var_dump(unserialize(serialize($n)));

try {
    unserialize('C:3:"GMP":0:{}');
} catch (Exception $e) { var_dump($e->getMessage()); }

try {
    unserialize('C:3:"GMP":8:{s:2:"42"}');
} catch (Exception $e) { var_dump($e->getMessage()); }

?>
