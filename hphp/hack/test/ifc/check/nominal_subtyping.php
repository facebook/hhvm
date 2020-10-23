<?hh

class Secret {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PRIVATE")>>
    public int $secret,
  ) {}
}

class A {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PUBLIC")>>
    public int $pa,
  ) {}
}

class B extends A {
  <<__InferFlows>>
  public function __construct(
    public int $pa
  ) {
    parent::__construct($pa);
  }
}

class C extends B {
  <<__InferFlows>>
  public function __construct(
    public int $pa
  ) {
    parent::__construct($pa);
  }
}

<<__InferFlows>>
function castBA(B $b): A { return $b; }

<<__InferFlows>>
function testBA(Secret $secret, B $b): void {
  castBA($b)->pa = $secret->secret;
}

<<__InferFlows>>
function castCA(C $c): A { return $c; }

<<__InferFlows>>
function testCA(Secret $secret, C $c): void {
  castCA($c)->pa = $secret->secret;
}
