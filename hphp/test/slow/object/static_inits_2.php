<?php

function __autoload($x) {
  global $y;
  $y = new stdclass;
  if (mt_rand()) {
    class Zoo {
      const Bar = 2;
    }
  } else {
    class Zoo {
      const Baz = 4;
    }
  }
}

class A {
  static $foo = Zoo::Bar;
  function k() {
    echo "ok\n";
  }
}

$z = new A;
$z->k();
var_dump($z);
var_dump($y);
