<?hh

function foo(inout $x) {
  return $x + $x;
}
<<__EntryPoint>> function main(): void {
$x = 1;
var_dump(foo(inout $x));
var_dump($x);
}
