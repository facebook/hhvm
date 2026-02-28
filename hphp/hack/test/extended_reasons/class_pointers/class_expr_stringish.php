<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_stringish<T as Stringish>(T $t): T { return $t; }

function test(): int {
  $gc = generic_stringish(C::class);
  return $gc;
}
