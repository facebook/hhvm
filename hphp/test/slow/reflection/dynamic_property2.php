<?hh // strict

class X {
  public $foo = 12;
}


<<__EntryPoint>>
function main_dynamic_property2() {
$x = new X;
$x->bar = 123;
$rc = new ReflectionClass('X');
var_dump($rc->getProperties());
}
