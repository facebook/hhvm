<?php

class Ex1 extends Exception {
}

class Ex2 extends Exception {
}

class Ex3 extends Exception {
}

function bar($a) {
  print "bar $a\n";
  $b = array(1, 2);
  foreach($b as $c) {
    if ($a == 0) {
      throw new Ex1();
    } else if ($a == 2) {
      throw new Ex2();
    } else if ($a == 3) {
      throw new Ex3();
    }
  }
}

function foo2($a) {
  try {
    call_user_func("bar", $a);
  } catch (Ex1 $e) {
    print "caught 1\n";
  }
}

function foo1($a) {
  foo2($a);
}

function foo($a) {
  foo1($a);
}

$a = array(0, 1, 2);
$b = array(0);

foreach ($b as $c) {
  try {
    array_map("foo", $a);
  } catch (Ex2 $e) {
    print "caught 2\n";
  }
}

try {
  foreach (array(1,2,3) as $_) {
    echo "before\n";
    throw new Exception();
    echo "after\n";
  }
} catch (Exception $e) {
  echo "caught\n";
}

class A {
  function __construct() {
    throw new Exception();
  }
}

try {
  call_user_func("hphp_create_object", "A", NULL);
} catch (Exception $e) {
  print "caught exception\n";
}

$b = array(3);
try {
  array_map("foo", $b);
} catch (Ex3 $e) {
  print "caught 3\n";
  throw $e;
}
