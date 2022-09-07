<?hh
namespace ROTDB\ScopeIf3;
class Foo {
  public int $x = 0;
}
function ro(): readonly Foo {
  return new Foo();
}
function ro2(): readonly Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  if (1 < 2) {
    $f = ro2<>;
  } else {
    $f = ro<>;
  }
  $foo = $f(); // inferred readonly
  $foo->x = 3; // fatal
}
