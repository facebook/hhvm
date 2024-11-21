<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_classname<T as classname<mixed>>(T $t): T { return $t; }

function test(class<C> $c): int {
  $gc = generic_classname($c);
  return $gc;
}
