<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_string<T as string>(T $t): T { return $t; }

function test(class<C> $c): int {
  $gc = generic_string($c);
  return $gc;
}
