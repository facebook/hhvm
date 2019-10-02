<?hh

function f(inout $x) { var_dump($x); }
function test($b, $c) {
  $x = false && $b;
  $x += true && $b;
  $x += false || $b;
  $x += true || $b;

  $x += false ? $b : $c;
  $x += true ? $b : $c;
  f(inout $x);
}
<<__EntryPoint>> function main(): void {
test(2, 3);
}
