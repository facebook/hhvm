<?hh

<<file:__EnableUnstableFeatures('require_constraint')>>

trait T1 {
  require this as C;
}

trait T2 {
  use T1;
}

class D {
  use T2;
}

<<__EntryPoint>>
function main(): void {
  new D();
  echo "D created\n";
}
