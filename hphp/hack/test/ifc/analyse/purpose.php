<?hh // strict

class D {
  public function __construct(public int $di) {}
}

class C {
  public function __construct(
    <<Policied("Secret")>>
    public int $ci,
    <<Policied("Secret")>>
    public D $d,
  ) { }

  public function getShallow1(): int {
    return $this->ci;
  }

  public function getShallow2(): D {
    return $this->d;
  }

  public function getDeep(): int {
    return $this->d->di;
  }

  public function writeDeep(): void {
    $this->d->di = 42;
  }
}
