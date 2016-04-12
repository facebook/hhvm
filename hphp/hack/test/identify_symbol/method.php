<?hh

class Foo {
  public function bar() {}
}

function test(Foo $foo) {
  $foo->bar();
}
