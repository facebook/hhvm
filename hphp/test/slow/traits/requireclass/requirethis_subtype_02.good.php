<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  use T;
}

trait T {
  require this as C;
}

<<__EntryPoint>>
function main(): void {
  (new C());
  echo "hello";
}
