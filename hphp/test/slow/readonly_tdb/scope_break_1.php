<?hh
namespace ROTDB\ScopeBreak1;
class Foo {
  public int $x = 0;
}
function ro(): readonly Foo {
  return new Foo();
}

function mut(): Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $f = mut<>;
  do {
    break;
    $f = ro<>;
  } while (false);
  $foo = $f(); // we don't infer readonly
  $foo->x = 3; // OK
}
