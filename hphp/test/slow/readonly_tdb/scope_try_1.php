<?hh
namespace ROTB\ScopeTry1;
class Foo {
  public int $x = 0;
}

function ro(): readonly Foo {
  return new Foo();
}

function mut(): Foo {
  return new Foo();
}

function maybe_throw(): void {}

<<__EntryPoint>>
function main(): void {
  $f = ro<>;
  $g = ro<>;
  try {
    if (true) {
      $g = mut<>;
      maybe_throw();
      $g = ro<>;
    }
  } catch (Exception $e) {
    $f = ro<>;
  }
  $_ = $f(); // inferred readonly
  $_ = $g(); // no inferred readonly, ReadonlyViolationException
}
