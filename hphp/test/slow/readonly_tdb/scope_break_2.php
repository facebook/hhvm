<?hh
namespace ROTDB\ScopeBreak2;
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
    if (1 < 2) {
      break;
    } else {
      $f = ro<>;
    }
  } while (false);

  // Typing[4412] This function call returns a readonly value....
  $foo = $f(); // we don't infer readonly here
  $foo->x = 3; // OK
}
