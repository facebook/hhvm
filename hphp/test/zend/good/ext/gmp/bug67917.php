<?php

$mem1 = memory_get_usage();
for ($i = 0; $i < 1000; $i++) {
    $gmp = gmp_init(42);
    $gmp <<= 1;
}
$mem2 = memory_get_usage();

var_dump($mem2 - $mem1 < 100000);

?>
