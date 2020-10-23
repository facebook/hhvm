<?hh

class D {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("pd")>>
    public int $pd
  ) {}
}

class E extends D {
  <<__InferFlows>>
  public function __construct(
    public int $pd,
    <<__Policied("pe")>>
    public int $pe,
  ) {
    parent::__construct($pd);
  }
}

<<__InferFlows>>
function testPoliciedPropertiesED(E $e): D { return $e; }

class F extends E {
  <<__InferFlows>>
  public function __construct(
    public int $pd,
    public int $pe,
    <<__Policied("pf")>>
    public int $pf,
  ) {
    parent::__construct($pd, $pe);
  }
}

<<__InferFlows>>
function testPoliciedPropertiesFE(F $f): E { return $f; }
<<__InferFlows>>
function testPoliciedPropertiesFD(F $f): D { return $f; }
