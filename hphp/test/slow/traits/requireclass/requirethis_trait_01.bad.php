<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

trait T2 {}

trait T {
  require this as T2;
}

class C implements I {
  use T;
}

<<__EntryPoint>>
function main(): void {
  (new C());
  echo "hello";
}
