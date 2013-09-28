<?php
$arr = array(1,2,3);
$arr["foo"] = array(4,5,6);
$copy = $arr;

unset($copy["foo"][0]);
print_r($arr);
print_r($copy);
?>