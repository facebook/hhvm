<?hh

function foo(&$x) {
  return $x + $x;
}
<<__EntryPoint>> function main(): void {
$x = 1;
var_dump(foo(&$x));
var_dump($x);
}
