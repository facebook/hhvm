<?php


<<__EntryPoint>>
function main_541() {
$GLOBALS['foo'] = 10;
$GLOBALS['bar'] = 
  array(
      10 => array($GLOBALS['foo']),
      20 => array($GLOBALS['foo']));
var_dump($GLOBALS['bar']);
}
