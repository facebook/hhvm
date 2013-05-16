<?php

$arr1 = array('a','b','c');
$arr2 = array();
$arr3 = array('c','key'=>'d');
$arr4 = array("a\0b"=>'e','key'=>'d', 'f');

var_dump(in_array(123, $arr2));
var_dump(in_array(123, $arr1));
var_dump(array_search(123, $arr1));
var_dump(in_array('a', $arr1));
var_dump(array_search('a', $arr1));
var_dump(array_search('e', $arr4));
var_dump(array_search('d', $arr4));

echo "Done\n";
?>