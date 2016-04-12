<?hh

class Foo {}

function f(?Foo $x, mixed $y): Foo {
  if ($y instanceof $x) {
    hh_show($y);
    return $y;
  }
  invariant_violation('');
}
