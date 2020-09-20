<?hh

class ReqExtendsClass {
  public function foo(): int {
    return 0;
  }
}

interface FooRequireExtends {
  require extends ReqExtendsClass;
}

function foo_test_require_extends(FooRequireExtends $req_extends_test) {
  $req_extends_test->foo(); // Find ref
}
