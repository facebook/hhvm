<?php

function foo() {
  global $x; $x++;
  global $x2; $x2 = 44;
  global $a; $a[] = "a";
  global $a2; $a2 = array("a2");
}

function bar() {
  global $x; var_dump($x);
  global $x2; var_dump($x2);
  global $a; var_dump($a);
  global $a2; var_dump($a2);
}

$x = 42;
$a = array();
foo();
bar();
