<?php
$array1 = array("a" => "green", "b" => "brown", "c" => "blue", "red", "");
$array2 = array("a" => "green", "yellow", "red", TRUE);
$array3 = array("red", "a"=>"brown", "");
$result[] = array_diff_key($array1, $array2);
$result[] = array_diff_key($array1, $array3);
$result[] = array_diff_key($array2, $array3);
$result[] = array_diff_key($array1, $array2, $array3);

var_dump($result);

?>