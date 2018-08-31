<?php


<<__EntryPoint>>
function main_516() {
$a = array(
  array('a' => 'a'),
  array('b' => 'bb'),
  array('c' => 'cc'),
);

$refs = array();
foreach ($a as &$arr) {
  $refs[] = &$arr;
}
array_splice($a, 1, 1);
var_dump($a);
}
