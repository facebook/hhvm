<?hh

function foo() {
  foo();
}

class C {
  public function __construct(
    public $x = foo(),
  ) {
    foo();
  }
}
