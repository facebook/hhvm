<?hh

type MyTypedef = int;

function test(dynamic $x): void {
  (MyTypedef)$x;
}
