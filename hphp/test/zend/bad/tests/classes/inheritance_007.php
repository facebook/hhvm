<?php
class A {
  public function B () { echo "In " . __METHOD__ . "\n"; }
  public function A () { echo "In " . __METHOD__ . "\n"; }
}
class B extends A { }

$rc = new ReflectionClass('B');
var_dump($rc->getMethods());


$b = new B();
$b->a();
$b->b();

?>