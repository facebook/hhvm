<?hh

type IntTypedef = int;

function test(): void {
  $x = 1;
  (bool)($x as IntTypedef);
}
