<?hh

abstract class FooParent {
  protected function __construct(private string $prop) {}
}

trait TBar {
  public function __construct() {
    // no require extends => no problem
  }
}

trait TFoo {
  require extends FooParent;

  public function __construct() {
    // BUG: there's no call to parent::__construct
  }
}

class FooChild extends FooParent {}

class FooGrandChild extends FooChild {
  use TFoo;
}

function do_test(): void {
  $x = new FooGrandChild();
}
