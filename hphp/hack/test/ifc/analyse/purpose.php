<?hh // strict

class D {
  <<__InferFlows>>
  public function __construct(public int $di) {}
}

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("Secret")>>
    public int $ci,
    <<__Policied("Secret")>>
    public D $d,
  ) { }

  <<__InferFlows>>
  public function getShallow1(): int {
    return $this->ci;
  }

  <<__InferFlows>>
  public function getShallow2(): D {
    return $this->d;
  }

  <<__InferFlows>>
  public function getDeep(): int {
    return $this->d->di;
  }

  <<__InferFlows>>
  public function writeDeep(): void {
    $this->d->di = 42;
  }
}
