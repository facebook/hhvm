<?php

function __autoload($x) {

  $GLOBALS['y'] = new stdclass;
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
  public static $foo = Zoo::Bar;
  function k() {
    echo "ok\n";
  }
}

$z = new A;
$z->k();
var_dump($z);
var_dump($y);
