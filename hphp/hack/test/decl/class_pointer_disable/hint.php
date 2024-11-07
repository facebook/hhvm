<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param(class<C> $c): void {}
function ret(): class<C> { return C::class; }
class C {
  public static ?class<C> $c = null;
}
