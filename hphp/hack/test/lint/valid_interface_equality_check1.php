<?hh // strict

interface A {}
interface B {}

class C implements A, B {}

function test(A $a, B $b): void {
  // It is possible for this comparison to evaluate to true even though there is
  // no subtyping relationship between A and B, since C exists.
  // We don't have a simple way of determining whether such a class exists at
  // lint time, so we don't warn about comparisons between two interface types.
  if ($a === $b) {
    var_dump($a);
  }
}
