<?hh

// Hhbbc better not remove any of the parameter locals just because they aren't
// used.
function foo($x = 0, $y, $z) :mixed{
  $y = 2;
  for (;;) { continue; }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
