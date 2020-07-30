<?hh // strict

// With respect to the implicitly constructed lattice:
//
//   PRIVATE
//  /       \
// A         B
//  \       /
//    PUBLIC

class C {
  public function __construct(
    <<Policied("A")>>
    public int $a,
    <<Policied("B")>>
    public int $b,
    <<Policied("PUBLIC")>>
    public int $public,
    <<Policied("PRIVATE")>>
    public int $private,
  ) {}

  public function test(): void {
    $this->public = $this->public;   // Ok
    $this->private = $this->public;  // Ok
    $this->private = $this->private; // Ok
    $this->private = $this->a;       // Ok
    $this->a = $this->public;        // Ok
    $this->a = $this->a;             // Ok

    $this->public = $this->private; // Not ok
    $this->a = $this->private;      // Not ok
    $this->b = $this->a;            // Not ok
  }
}
