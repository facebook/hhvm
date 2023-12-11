<?hh
/* Prototype  : mixed array_reduce(array input, mixed callback [, int initial])
 * Description: Iteratively reduce the array to a single value via the callback.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */


function oneArg($v) :mixed{
  return $v;
}

function threeArgs($v, $w, $x) :mixed{
  return $v + $w + $x;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reduce() : variation ***\n";

$array = vec[1];

echo "\n--- Testing with a callback with too few parameters ---\n";
var_dump(array_reduce($array, oneArg<>, 2));

echo "\n--- Testing with a callback with too many parameters ---\n";
try { var_dump(array_reduce($array, threeArgs<>, 2)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "===DONE===\n";
}
