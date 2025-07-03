<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {}

trait T {
  require this as C;
}

class D {
  use T;
}

<<__EntryPoint>>
function main(): void {
  new D();
  echo "D created\nn";
}
