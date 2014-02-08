<?php
class A {
  public function fetchAll() {
  }
}

$reflection = new ReflectionClass('A');
var_dump($reflection->getMethods());
var_dump(get_class_methods('A'));

$reflection = new ReflectionClass('PDOStatement');
var_dump($reflection->getMethods());
var_dump(get_class_methods('PDOStatement'));
?>
