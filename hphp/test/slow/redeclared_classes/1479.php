<?php

if (!isset($g2)) {
  class test {
}
}
 else {
  class test {
    static $foo = 27;
  }
  var_dump(test::$foo);
}
$x = new test();
$x->bar = 1;
$x->foo = 2;
var_dump($x);
