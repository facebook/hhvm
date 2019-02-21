<?hh // partial
class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f(): void {
  $a = HH\Rx\mutable(new A(10));
  $a->x = 100;
  $b = vec[new A(42)];
  foreach($b as $k => $a)
  {
    // ERROR
    $a->x = 42;
  }
}
