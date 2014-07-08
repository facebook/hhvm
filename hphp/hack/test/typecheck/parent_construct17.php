<?hh // strict

abstract class FooParent {
  protected function __construct() {}
}

trait TFoo {
  require extends FooParent;

  public function __construct() {
    parent::__construct();
  }
}

class FooChild extends FooParent {}

class FooGrandChild extends FooChild {
  use TFoo;
}

function do_test(): void {
  $x = new FooGrandChild();
}
