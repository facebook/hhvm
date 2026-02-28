<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param(class<C> $c, enum<E> $e): void {
  hh_show($c);
  hh_show($e);
}
function ret(): class<C> { return C::class; }
function ret_enum(): enum<E> { return E::class; }
class C {
  public static ?class<C> $c = null;
  public static ?enum<E> $e = null;
}
enum E: int {
  A = 1;
}

function main(): void {
  hh_show(ret());
  hh_show(C::$c);
  hh_show(C::$e);
}
