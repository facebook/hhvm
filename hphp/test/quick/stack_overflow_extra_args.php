<?hh

function foo($x, $y) {
  foo($x, $y, $x + $y, $y + $x);
}
<<__EntryPoint>> function main(): void {
foo(1, 2);
}
