<?hh

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("PUBLIC")>>
    public int $cx,
    public int $cy,
    public D $cd,
  ) { }

  <<__InferFlows>>
  public function testGetUnpolicied(): D {
    return $this->cd;
  }

  <<__InferFlows>>
  public function testSetMultipleUnpolicied(D $d): void {
    $this->cy = 42;
    $this->cd = $d;
  }

  <<__InferFlows>>
  public function testSetDeep(int $i): void {
    $this->cd->di = $i;
  }
}

class D {
  <<__InferFlows>>
  public function __construct(public int $di) {}
}
