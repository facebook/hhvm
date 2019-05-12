<?hh

function foo($x) {
  $x = $x + 1;
  return $x;
}

function main() {
  $y = foo(1);
  return $y;
}
<<__EntryPoint>> function main_entry(): void {
var_dump(main());
}
