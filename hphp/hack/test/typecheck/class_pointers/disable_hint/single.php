<?hh

<<file:__EnableUnstableFeatures('class_type')>>

function param(class<C> $c): void {
  hh_show($c);
}
function ret(): class<C> { return C::class; }
class C {
  public static ?class<C> $c = null;
}

function main(): void {
  hh_show(ret());
  hh_show(C::$c);
}
