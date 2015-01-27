<?php

$arr = [gmp_init(0), -3, gmp_init(2), 1];
sort($arr);
var_dump($arr);

var_dump(min(gmp_init(3), 4));
var_dump(max(gmp_init(3), 4));

?>
