<?hh

/* Compile only: verify no c++ compilation errors */function foo($a) {
  return $a[1];
}
function baz(inout $x) {
 if ($x) $x++;
 }
function bar($a) {
  baz(foo($a)[1]);
  foo($a)->bar = 1;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
