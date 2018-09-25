<?hh
class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f(Map<A, int> $g): void {
  $k = HH\Rx\mutable(new A(10));
  $k->x = 100;
  foreach ($g as $k => $a)
  {
    // ERROR
    $k->x = 42;
  }
}
