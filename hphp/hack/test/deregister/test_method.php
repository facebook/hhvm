<?hh // strict


class Foo {
  <<__PHPStdLib>>
  public function foo() : void {

  }


  public function bar() : void {

  }
}

function test() : void {
  $x = new Foo();
  $x->bar();
  $x->foo();
}
