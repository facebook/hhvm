<?hh // strict

interface A {}

class C {}

class D extends C implements A {}

function test(A $a, C $c): void {
  // It is possible for this comparison to evaluate to true even though there is
  // no subtyping relationship between A and C, since D exists.
  // We don't have a simple way of determining whether such a class exists at
  // lint time, so we don't warn about comparisons involving an interface type.
  if ($a === $c) {
    var_dump($a);
  }
}
