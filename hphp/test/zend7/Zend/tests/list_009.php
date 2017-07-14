<?php

$a = [1];
[&$foo] = $a;
$foo = 2;

var_dump($a);

?>
