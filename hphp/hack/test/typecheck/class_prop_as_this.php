<?hh // strict

// The test proves that it is safe to allow the 'this' type as a public
// property. Attempting to assign to a property of type 'this' will fail
// if they are not the same expression dependent type.
abstract class C {
  public function __construct(public this $x) {}

  public function test(C $c1, C $c2, this $static): void {
    // This works because both are known to be of type <static>
    $static->x = $this->x;
    hh_show($static->x);
    hh_show($this->x);

    // But since we don't know if $c1 and $c2 refer to the same type this is an
    // error.
    $c1->x = $c2->x;
  }
}
