<?php

$var = "intergalactic";
$var1 = &$var;
$var = $var[5];

var_dump($var);
var_dump($var1);

echo "Done\n";
?>