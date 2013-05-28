<?php

function handler($errno, $errstr) {
  var_dump($errno);
  return true;
}
set_error_handler('handler');
unserialize();
define();
define('u');
define('a','X');
define('b','Y',false);
define('c',1,2,3,4,foo());
var_dump(a,b,c);
var_dump(defined('a'),defined('b'),defined('c'));
function foo() {
  var_dump('FOO');
}
