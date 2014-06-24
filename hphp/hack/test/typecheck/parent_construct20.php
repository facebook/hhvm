<?hh

abstract class FooGrandParent {
  protected string $foo;
}

abstract class FooParent extends FooGrandParent {
  public function __construct(string $foo) {
    $this->foo = $foo;
  }
}

trait FooT {
  require extends FooGrandParent;
}

class Foo extends FooParent {
  use FooT;
}

function do_test(): void {
  $x = new Foo('foo');
}
