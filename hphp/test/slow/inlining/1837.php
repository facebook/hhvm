<?hh

/* Compile only: verify no c++ compilation errors */function foo($a) :mixed{
  return $a[1];
}
function baz(inout $x) :mixed{
 if ($x) $x++;
 }
function bar($a) :mixed{
  baz(foo($a)[1]);
  foo($a)->bar = 1;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
