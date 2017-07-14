<?php

$a = 1;
$fn = function() use ($a, &$a) {
    $a = 2;
};
$fn();
var_dump($a);

?>
