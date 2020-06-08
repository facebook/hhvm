<?hh // strict

class C {
  <<Policied>>
  public int $cx = 0;
  public int $cy = 0;

  public function __construct(public D $cd) { }

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
