<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  use T;

  public static function foo(): void { echo "I am foo\n"; }
}

trait T {
  require this as C;
}
<<__EntryPoint>>
function main(): void {
  T::foo();
}
