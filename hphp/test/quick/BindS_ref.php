<?php

class Foo {
  public static $x;

  function f() {
    $x = array();
    Foo::$x =& $x;
    Foo::$x[0] = 12;
    echo $x[0] . "\n";
  }
}
$y = new Foo();
$y->f();
echo "done\n";
