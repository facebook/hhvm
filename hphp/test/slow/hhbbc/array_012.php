<?hh

// Potentially an infinitely-growing map type.
function foo($x) :mixed{ return $x ? dict['x' => foo()] : dict['y' => foo()]; }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
