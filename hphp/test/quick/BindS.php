<?php

class Foo {
  public static $x;
}

function f() {
  $x = 2;
  Foo::$x = &$x;
}
f();
echo "done\n";
