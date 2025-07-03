<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

interface I {}

trait T {
  require extends I;
}

class C implements I {
  use T;
}

<<__EntryPoint>>
function main(): void {
  (new C());
  echo "hello";
}
