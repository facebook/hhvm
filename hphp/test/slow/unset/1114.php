<?php

$a = 1;
function foo() {
  $GLOBALS['foo'] = 1;
  unset($GLOBALS['foo']);
  var_dump(array_key_exists('foo', $GLOBALS));
  $g['foo'] = 1;
  unset($g['foo']);
  var_dump(array_key_exists('foo', $g));
  var_dump(array_key_exists('a', $GLOBALS));
  unset($GLOBALS['a']);
  var_dump(array_key_exists('a', $GLOBALS));
}
foo();
