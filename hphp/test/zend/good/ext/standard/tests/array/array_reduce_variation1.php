<?hh
/* Prototype  : mixed array_reduce(array input, mixed callback [, int initial])
 * Description: Iteratively reduce the array to a single value via the callback.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */


function oneArg($v) {
  return $v;
}

function threeArgs($v, $w, $x) {
  return $v + $w + $x;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reduce() : variation ***\n";

$array = varray[1];

echo "\n--- Testing with a callback with too few parameters ---\n";
var_dump(array_reduce($array, fun("oneArg"), 2));

echo "\n--- Testing with a callback with too many parameters ---\n";
try { var_dump(array_reduce($array, fun("threeArgs"), 2)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "===DONE===\n";
}
