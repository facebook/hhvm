<?hh
namespace ROTDB\ScopeSwitch1;
class Foo {
  public int $x = 0;
}

function ro(): readonly Foo {
  return new Foo();
}

function mu(): Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $a = $a = 2;
  $f = ro<>;
  for ($i = 0; $i < 2; $i++) {
    switch ($i) {
      case 0:
        $f = mu<>;
        break;
      case 1:
        $_ = $f(); // inferred readonly
        $f = ro<>;
      default:
        $_ = $f(); // inferred readonly
        $f = ro<>;
    }
  }
  // we don't infer readonly here because
  // $f does not return a readonly value in all branches
  $_ = $f(); // ReadonlyViolationException
}
