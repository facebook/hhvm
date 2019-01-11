<?php

<<__EntryPoint>>
function main_array_multisort_bug_1() {
error_reporting(-1);
$arr1 = array(3, 1, 2);
$arr2 = array('a', 'b', 'c');
var_dump(array_multisort(&$arr1, &$arr2));
var_dump($arr2);
}
