<?php
$tmp = array();
$tmp['foo'] = "test";
$tmp['foo'] = &bar($tmp['foo']);
var_dump($tmp);

unset($tmp);

$tmp = array();
$tmp['foo'] = "test";
$tmp['foo'] = &fubar($tmp['foo']);
var_dump($tmp);

function bar($text){
  return $text;
}

function fubar($text){
  $text = &$text;
  return $text;
}
?>