<?php
function foo() {
  if (isset($GLOBALS['b'])) $b = 0;
  return $b;
}
foo();

function baz($x) {}
function bar() {
  if (isset($GLOBALS['a'])) $a = 0;
  baz($a);
}
bar();
