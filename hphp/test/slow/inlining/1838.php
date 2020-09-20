<?hh

/* Compile only: verify no c++ compilation errors */function f($x) {
  return function () use ($x) {
    return $x;
  }
;
}
function g($x) {
  $c = f($x);
  return $c();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
