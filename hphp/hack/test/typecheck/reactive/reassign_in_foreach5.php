<?hh // partial
class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f(Map<int, A> $g): void {
  $a = HH\Rx\mutable(new A(10));
  $a->x = 100;
  foreach ($g as $k => $a)
  {
    // ERROR
    $a->x = 42;
  }
}
