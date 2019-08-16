<?hh

// This tests to make sure that the typedef's generics are put in the environment
type X<Tcollide> = Tcollide;
type Y<Tignore> = int;

function f(X<int> $x): void {}

function g(Y<null> $y): void {}

function test<
  <<__Enforceable>> reify Tcollide
>(dynamic $d): void {
  f($d); // error
  g($d);
}
