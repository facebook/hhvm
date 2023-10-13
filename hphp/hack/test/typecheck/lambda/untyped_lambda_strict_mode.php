<?hh

function expect_mixed(mixed $f): void {}
function test(): void {
  expect_mixed($x ==> $x->foo());
}
