<?hh
<<__EntryPoint>> function main(): void {
var_dump(function() { } is closure);
var_dump(function(&$x) { } is closure);
var_dump(@function(&$x) use ($y, $z) { } is closure);
}
