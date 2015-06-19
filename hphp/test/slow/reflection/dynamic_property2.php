<?hh // strict

Class X {
  public $foo = 12;
}

$x = new X;
$x->bar = 123;
$rc = new ReflectionClass('X');
var_dump($rc->getProperties());
