<?php

$a = gmp_init(3);
$b = clone $a;
gmp_clrbit($a, 0);
var_dump($a, $b); // $b should be unaffected

?>
