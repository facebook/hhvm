<?hh // strict

// This test that using the 'this' type as an argument to a public instance
// method is safe.
abstract class C {
  public function __construct(private this $self) {}
  public function set(this $x): void {
    $this->self = $x;
  }

  public function test(C $c1, C $c2, this $self): void {
    $self->set($this);
    $this->set($self);

    // Error because $c1 and $c2 may have different late-bound types
    $c2->set($c1);
  }
}
