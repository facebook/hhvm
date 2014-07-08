<?php

$a = array();
$s = "";
var_dump(array_unshift($a, $s));
var_dump($a);
var_dump(array_unshift($s, $a));
var_dump($a);
var_dump(array_unshift($a, $a));
var_dump($a);

echo "Done\n";
?>