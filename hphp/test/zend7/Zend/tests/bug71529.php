<?php

$a = [1];
$a[] = &$a[out(count($a) - 1)];
var_dump($a);

function out($what) {
    var_dump($what);
    return $what;
}

?>
