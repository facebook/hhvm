<?hh

function foo($x) {
  if (!$x) { return true; }
  else { return false; }
}
<<__EntryPoint>> function main(): void {
var_dump(foo(true));
var_dump(foo(1));
}
