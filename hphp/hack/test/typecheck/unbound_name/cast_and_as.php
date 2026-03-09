<?hh

type IntTypedef = int;

function test(): void {
  $x = 1;
  (string)($x as IntTypedef);
}
