<?hh // strict

class A {
  public function __construct(private string $val) {}

  public function broken(C $c): string {
    return $c->val;
  }
}

class B extends A {
  public function __construct(string $val) {
    parent::__construct($val);
  }
}
class C extends B {
  public function __construct(string $s, public string $val) {
    parent::__construct($s);
  }
}
