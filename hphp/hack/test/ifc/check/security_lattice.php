<?hh

// With respect to the implicitly constructed lattice:
//
//   PRIVATE
//  /       \
// A         B
//  \       /
//    PUBLIC

class C {
  <<__InferFlows>>
  public function __construct(
    <<__Policied("A")>>
    public int $a,
    <<__Policied("B")>>
    public int $b,
    <<__Policied("PUBLIC")>>
    public int $public,
    <<__Policied("PRIVATE")>>
    public int $private,
  ) {}

  <<__InferFlows>>
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
