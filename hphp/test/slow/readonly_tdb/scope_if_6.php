<?hh
namespace ROTDB\ScopeIf6;
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
  $f = ro<>;
  if (true) {
    if (true) {
      $f = ro<>;
      if (true) {
        do {
          // only place where we assign
          // a non-readonly-returning function to $f
          $f = mut<>;
        } while (false);
      } else {
        $f = ro<>;
      }
    } else {
      $f = ro<>;
    }
  }
  $foo = $f(); // we don't infer readonly
  $foo->x = 3; // OK
}
