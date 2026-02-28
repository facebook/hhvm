<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_stringish<T as Stringish>(T $t): T { return $t; }

function test(class<C> $c): int {
  $gc = generic_stringish($c);
  return $gc;
}
