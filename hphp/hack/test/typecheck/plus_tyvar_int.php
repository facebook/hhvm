<?hh

function expect_int(int $n) : void {
}

function f<T as num>(T $n) : void {
  $m = Vector {};
  $m[] = $n;
  expect_int($m[0] + 1);
}

<<__EntryPoint>>
function main() : void {
  f(1.1);
}
