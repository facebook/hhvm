<?hh

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
  $g = mut<>;
  function() use ($f, $g) {
    $f = mut<>;
    $g = ro<>;
  }();
  () ==> {
    $f = mut<>;
    $g = ro<>;
  }();
  $goo = $g(); // no inferred readonly
  $goo->x = 3; // OK
  $foo = $f(); // inferred readonly
  $foo->x = 3; // fatal
}
