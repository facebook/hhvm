<?hh
<<file:__EnableUnstableFeatures('ifc')>>

function foo(int $x): void {
  ($x ==> $x) ($x);
}
