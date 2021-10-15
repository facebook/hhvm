<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {
  public int $prop = 5;
}
<<__EntryPoint>>
function foo(): void {
  $a = vec[new Foo()];
  $z = hphp_array_idx($a, 0);
  $z->prop = 5; // ok
  $x = readonly vec[new Foo(), new Foo()];
  $y = hphp_array_idx($x, 0);
  $y[0]->prop = 5;
}
