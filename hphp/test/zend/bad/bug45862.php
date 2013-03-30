<?php

class Ancestor {
  function test() {
    var_dump(get_class_vars("Tester"));
    var_dump(Tester::$prot);
  }
}

class Tester extends Ancestor {
  static protected $prot = "protected var";
  static private $priv = "private var";
}

class Child extends Tester {
  function test() { var_dump(get_class_vars("Tester")); }
}

echo "\n From parent scope\n";
$parent = new Ancestor();
$parent->test();
echo "\n From child scope\n";
$child = new Child();
$child->test();

?>