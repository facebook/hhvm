<?hh
class Foo {
  public int $prop = 5;
}
<<__EntryPoint>>
  function foo(): void {
    $x = readonly Vector {new Foo()};
    $y = vec($x);
    $y[0]->prop = 5;
  }
