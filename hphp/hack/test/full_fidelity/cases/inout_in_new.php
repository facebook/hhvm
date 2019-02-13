<?hh // strict

class Foo {
  public function __construct(&$bar) {}
}

function foo($bar) {
  new Foo(inout $bar);
  PHP\var_dump($bar);
}
