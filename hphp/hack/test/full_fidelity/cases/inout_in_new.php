<?hh

class Foo {
  public function __construct(inout $bar) {}
}

function foo($bar) {
  new Foo(inout $bar);
  PHP\var_dump($bar);
}
