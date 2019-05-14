<?hh

trait T {
  protected $params;
};

class B {
  protected $params;
};

class B1 extends B {
  use T;
}

class B2 extends B {
  use T;
}

class C extends B1 {
  function __construct($b2) {
    $this->params = $b2->params;
  }
}
<<__EntryPoint>> function main(): void {
var_dump(new C(new B2));
}
