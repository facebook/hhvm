<?hh

class A<T> { }

class B<T> extends A<T> {}

function testTParams1(B<int> $b): A<int> { return $b; }

class C<T> extends A<int> {}

function testTParams2(C<nothing> $c): A<int> { return $c; }

class D {
  public function __construct(
    <<Policied>>
    public int $pd
  ) {}
}

class E extends D {
  public function __construct(
    public int $pd,
    <<Policied>>
    public int $pe,
  ) {
    parent::__construct($pd);
  }
}

function testPoliciedPropertiesED(E $e): D { return $e; }

class F extends E {
  public function __construct(
    public int $pd,
    public int $pe,
    <<Policied>>
    public int $pf,
  ) {
    parent::__construct($pd, $pe);
  }
}

function testPoliciedPropertiesFE(F $f): E { return $f; }
function testPoliciedPropertiesFD(F $f): D { return $f; }
