<?hh // strict

class Foo {}

<<__Rx>>
function some_function(<<__OwnedMutable>>Foo $foo, bool $c1, bool $c2): int {
  if ($c1) {
    return some_other_function(\HH\Rx\freeze($foo));
  }
  if ($c2) {
    try {
      return g();
    } catch (Exception $ex) {
      return some_other_function(\HH\Rx\freeze($foo));
    }
  }
  return 4;
}

<<__Rx>>
function g(): int {
  return 42;
}

<<__Rx>>
function some_other_function(Foo $foo): int {
  return 7;
}
