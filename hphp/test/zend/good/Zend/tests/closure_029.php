<?hh
<<__EntryPoint>> function main(): void {
var_dump(function() { } is closure);
var_dump(function(inout $x) { } is closure);
var_dump(@function(inout $x) use ($y, $z) { } is closure);
}
