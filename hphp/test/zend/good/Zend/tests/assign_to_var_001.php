<?php

$var = array(1,2,3);
$var1 = &$var;
$var = $var[1];

var_dump($var);
var_dump($var1);

echo "Done\n";
?>