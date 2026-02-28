<?hh

<<file:__EnableUnstableFeatures('class_type')>>

class C {}
function generic_string<T as string>(T $t): T { return $t; }

function test(): int {
  $gc = generic_string(C::class);
  return $gc;
}
