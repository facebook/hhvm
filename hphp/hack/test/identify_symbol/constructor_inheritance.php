<?hh

class C {
  public function __construct() {}
}

class Foo extends C {}

function test() {
  new Foo();
}
