<?hh

type IntTypedef = int;

function test(): void {
  $x = 1;
  (int)($x is IntTypedef);
}
