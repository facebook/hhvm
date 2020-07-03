<?hh // strict

// Assume a lattice with A < B; B < C; A < D

class C {
  public function __construct(
    <<Policied("A")>>
    public int $a,
    <<Policied("B")>>
    public int $b,
    <<Policied("C")>>
    public int $c,
    <<Policied("D")>>
    public int $d,
    <<Policied("PUBLIC")>>
    public int $public,
    <<Policied("PRIVATE")>>
    public int $private,
  ) {}

  public function testTopBottom(): void {
    $this->public = $this->public;
    $this->private = $this->public;
    $this->private = $this->private;
    $this->private = $this->a;
    $this->private = $this->c;
    $this->a = $this->public;
    $this->c = $this->public;
  }

  public function testLatticeProper(): void {
    $this->d = $this->a; // Ok
    $this->c = $this->b; // Ok
    $this->b = $this->a; // Ok
    $this->c = $this->a; // Ok
    $this->a = $this->a; // Ok

    $this->d = $this->c;  // Not ok
    $this->a = $this->c;  // Not ok
    $this->a = $this->b;  // Not ok
  }
}
