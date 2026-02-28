<?hh

// This tests that using the 'this' type as an argument to a public instance
// method is safe.
abstract class C {
  public function __construct(private this $self) {}
  public function set(this $x): void {
    $this->self = $x;
  }

  public function test(C $c1, C $c2, this $self): void {
    // 'this' as a method parameter is treated as the 'static' type when
    // type checking within the given method. However when the method
    // is invoked we generate an expression dependent type to ensure that
    // the 'this' type between two instance refer to exactly the same type.
    $self->set($this);
    $this->set($self);

    // Error because $c1 and $c2 may have different late-bound types
    $c2->set($c1);
  }
}
