<?hh

// Potentially an infinitely-growing map type.
function foo($x) :mixed{ return $x ? darray['x' => foo()] : darray['y' => foo()]; }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
