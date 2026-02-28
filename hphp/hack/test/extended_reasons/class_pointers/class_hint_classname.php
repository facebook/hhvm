<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_classname<T as classname<mixed>>(T $t): T { return $t; }

function test(): int {
  $gc = generic_classname(C::class);
  return $gc;
}
