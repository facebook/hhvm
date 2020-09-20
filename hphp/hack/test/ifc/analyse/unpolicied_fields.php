<?hh // strict

class C {
  public function __construct(
    <<Policied("PUBLIC")>>
    public int $cx,
    public int $cy,
    public D $cd,
  ) { }

  public function testGetUnpolicied(): D {
    return $this->cd;
  }

  public function testSetMultipleUnpolicied(D $d): void {
    $this->cy = 42;
    $this->cd = $d;
  }

  public function testSetDeep(int $i): void {
    $this->cd->di = $i;
  }
}

class D {
  public function __construct(public int $di) {}
}
