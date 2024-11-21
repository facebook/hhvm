<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param(class<C> $c, enum<E> $e): void {}
function ret(): class<C> { return C::class; }
function ret_enum(): enum<E> { return E::class; }
enum E: int {
  A = 1;
}
class C {
  public static ?class<C> $c = null;
  public static ?enum<E> $e = null;
}
