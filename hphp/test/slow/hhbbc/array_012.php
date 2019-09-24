<?hh

// Potentially an infinitely-growing map type.
function foo($x) { return $x ? array('x' => foo()) : array('y' => foo()); }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
