<?hh // strict

class A {
  <<__Policied("X")>>
  public int $x = 0;

  <<__Policied("Y")>>
  public int $y = 0;
};

<<__InferFlows>>
function g(A $a): void {
  $x = $a;
  if ($a->x > 0) {
    $x = null;
  }
  // $x: (A | null<X>)
  if ($x) {
    // the test's outcome depends on the policies
    // in the union type; in particular it depends
    // on X; so there is a bogus flow from X to Y
    $a->y = 1;
  }
}
