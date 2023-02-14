<?hh

function foo() {
  foo();
  foo<>;
}

class C {
  public function __construct(
    public $x = foo(),
  ) {
    foo();
    foo<>;
  }
}

function test() {
  foo();
  foo<>;
}
