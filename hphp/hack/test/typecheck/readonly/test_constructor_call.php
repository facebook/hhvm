<?hh
class Bar {}
class Foo {
  public function __construct(public mixed $foo) {
  }
}

function test(readonly Bar $y) : void {
  new Foo($y);
}
