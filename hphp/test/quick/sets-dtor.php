<?php

// Test destruction order for SetS.
class d {
  static public $x = "asd";

  public function __destruct() {
    d::$x = "destructor";
  }

  public static function foo() {
    $foo = new d();
    d::$x = $foo;
    unset($foo);
    echo "Foo: ";
    echo (d::$x = "main");
    echo "\n";
    var_dump(d::$x);
  }
}

d::foo();
