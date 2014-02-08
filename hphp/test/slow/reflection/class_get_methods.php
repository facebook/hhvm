<?php
class A {
  public function fetchAll() {
  }
}

$reflection = new ReflectionClass('A');
var_dump($reflection->getMethods());
var_dump(get_class_methods('A'));

$reflection = new ReflectionClass('PDOStatement');
$methods = $reflection->getMethods();
var_dump(array_map(function($x) { return $x->class . '::' . $x->name; },
  $methods));
var_dump(get_class_methods('PDOStatement'));
?>
