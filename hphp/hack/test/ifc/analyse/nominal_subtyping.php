<?hh

class D {
  public function __construct(
    <<Policied("pd")>>
    public int $pd
  ) {}
}

class E extends D {
  public function __construct(
    public int $pd,
    <<Policied("pe")>>
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
    <<Policied("pf")>>
    public int $pf,
  ) {
    parent::__construct($pd, $pe);
  }
}

function testPoliciedPropertiesFE(F $f): E { return $f; }
function testPoliciedPropertiesFD(F $f): D { return $f; }
