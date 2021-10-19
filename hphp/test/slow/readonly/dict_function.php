<?hh
class Foo {
  public int $prop = 5;
}
<<__EntryPoint>>
function foo(): void {
  $x = readonly Map { 0 => readonly new Foo() };
  $y = \HH\dict($x);
  $y[0]->prop = 5;
}
