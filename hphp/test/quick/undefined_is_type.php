<?php

function foo() {
  if (isset($GLOBALS['a'])) $a = 1;
  echo "set:     ", isset($a)      . "\n";
  echo "nul:     ", is_null($a)    . "\n";
  echo "str:     ", is_string($a)  . "\n";
  echo "obj:     ", is_object($a)  . "\n";
  echo "arr:     ", is_array($a)   . "\n";
  echo "int:     ", is_int($a)     . "\n";
  echo "integer: ", is_integer($a) . "\n";
  echo "long:    ", is_long($a)    . "\n";
  echo "real:    ", is_real($a)    . "\n";
  echo "double:  ", is_double($a)  . "\n";
  echo "float:   ", is_float($a)   . "\n";
  echo "bool:    ", is_bool($a)    . "\n";
}

foo();
