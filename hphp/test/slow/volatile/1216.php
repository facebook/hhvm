<?php

function bar() {
  for ($i = 0;
 $i < 4;
 $i++) {
    if ($i > 1 && !function_exists('foo')) {
      function foo() {
        echo "foo called\n";
      }
    }
    $foo = 'foo';
    if (function_exists($foo)) {
      foo();
    }
 else {
      echo "foo does not exists\n";
    }
  }
  for ($i = 0;
 $i < 4;
 $i++) {
    if ($i > 1 && !class_exists('goo')) {
      class goo {
        function goo() {
          echo "goo called\n";
        }
      }
    }
    $goo = 'goo';
    if (class_exists($goo)) {
      $a = new goo();
    }
 else {
      echo "goo does not exists\n";
    }
  }
}
if (function_exists('bar')) bar();
$a = 'bar';
if (function_exists($a)) bar();
$a = 'later';
if (function_exists($a)) {
  echo "later exists\n";
}
 else {
  echo "later does not exists\n";
}
$a = 'later2';
if (class_exists($a)) {
  echo "later2 exists\n";
}
 else {
  echo "later2 does not exists\n";
}
$a = 'later3';
if (function_exists($a)) {
  echo "later3 exists\n";
}
 else {
  echo "later3 does not exists\n";
}
$a = 'later4';
if (class_exists($a)) {
  echo "later4 exists\n";
}
 else {
  echo "later4 does not exists\n";
}
function later3() {
  echo "later3 called\n";
}
class later4 {
}
if (function_exists('function_exists')) {
  echo "yes\n";
}
if (class_exists('exception')) {
  echo "yes\n";
}
