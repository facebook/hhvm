<?hh
<<__EntryPoint>> function main(): void {
var_dump(function() { } is Closure);
var_dump(function(inout $x) { } is Closure);
try {
  var_dump(@function(inout $x) use ($y, $z) { } is Closure);
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
}
