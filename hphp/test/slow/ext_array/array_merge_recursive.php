<?php


<<__EntryPoint>>
function main_array_merge_recursive() {
$a1 = array();
$a2 = array("key1" => null);
$a1 = array_merge_recursive($a1, $a2);
unset($a1);unset($a2);

$ar1 = array(
  "color" => array("favorite" => "red"),
  1 => 5
);
$ar2 = array(
  "color" => array("favorite" => "green"),
  "blue"
);

$result = array_merge_recursive($ar1, array($ar2));
var_dump($result);
}
