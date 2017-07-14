<?php

function inc(&... $args) {
    foreach ($args as &$arg) {
        $arg++;
    }
}

$arr = [1, 2];
$arr[] = 3;
$arr2 = $arr;
inc(...$arr);
var_dump($arr);
var_dump($arr2);

?>
