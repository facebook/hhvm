<?hh

function foo() {
  foo();
  fun('foo');
}

class C {
  public function __construct(
    public $x = foo(),
  ) {
    foo();
    fun('foo');
  }
}

function test() {
  foo();
  fun('foo');
}
