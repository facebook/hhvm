<?php
class test implements itestable {
  public $foo;
  protected $prop;
  private $bar;
  public function foo() {}
  protected function func() {}
  function bar() {}
  const const_foo = 'f';
}

interface itestable {
  function foo();
  function bar();
}

$classes = get_declared_classes();
var_dump(in_array("test", $classes, true));

$interfaces = get_declared_interfaces();
var_dump(in_array("itestable", $interfaces, true));

var_dump(class_exists("TEst"));

var_dump(interface_exists("iTESTable"));

var_dump(get_class_methods("TEst")[0] === "foo");

var_dump(get_class_vars("TEst") === array("foo" => null));

var_dump(get_class_constants("test") === array("const_foo" => "f"));
