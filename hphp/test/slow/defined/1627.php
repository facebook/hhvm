<?php

function handler($errno, $errstr) {
  var_dump($errno);
  return true;
}
function foo() {
  var_dump('FOO');
}

<<__EntryPoint>>
function main_1627() {
set_error_handler('handler');
unserialize();
define();
define('u');
define('a','X');
define('b','Y',false);
define('c',1,2,3,4,foo());
var_dump(a,b,c);
var_dump(defined('a'),defined('b'),defined('c'));
}
