<?hh

/* Compile only: verify no c++ compilation errors */function f($x) :mixed{
  return function () use ($x) {
    return $x;
  }
;
}
function g($x) :mixed{
  $c = f($x);
  return $c();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
