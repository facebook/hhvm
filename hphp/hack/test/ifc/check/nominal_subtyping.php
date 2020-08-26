<?hh

class Secret {
  public function __construct(
    <<Policied("PRIVATE")>>
    public int $secret,
  ) {}
}

class A {
  public function __construct(
    <<Policied("PUBLIC")>>
    public int $pa,
  ) {}
}

class B extends A {
  public function __construct(
    public int $pa
  ) {
    parent::__construct($pa);
  }
}

class C extends B {
  public function __construct(
    public int $pa
  ) {
    parent::__construct($pa);
  }
}

<<InferFlows>>
function castBA(B $b): A { return $b; }

function testBA(Secret $secret, B $b): void {
  castBA($b)->pa = $secret->secret;
}

<<InferFlows>>
function castCA(C $c): A { return $c; }

function testCA(Secret $secret, C $c): void {
  castCA($c)->pa = $secret->secret;
}
