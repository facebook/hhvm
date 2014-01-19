<?php

for ($i = 0;
 $i < 4;
 $i++) {
  if ($i > 1 && !defined('CON')) {
    define(/*|Dynamic|*/'CON', 1);
  }
  if (defined('CON')) {
    var_dump(CON);
  }
 else {
    echo "CON does not exists\n";
  }
}
for ($i = 0;
 $i < 4;
 $i++) {
  if ($i > 1 && !function_exists('foo')) {
    function foo() {
      echo "foo called\n";
    }
  }
  if (function_exists('foo')) {
    foo();
  }
 else {
    echo "foo does not exists\n";
  }
}
for ($i = 0;
 $i < 4;
 $i++) {
  if ($i > 1 && !class_exists('bar')) {
    class bar {
      function bar() {
 echo "bar called\n";
 }
    }
  }
  if (class_exists('bar')) {
    $a = new bar;
  }
 else {
    echo "bar does not exists\n";
  }
}
