<?hh
/* Prototype  : mixed array_reduce(array input, mixed callback [, int initial])
 * Description: Iteratively reduce the array to a single value via the callback.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

class A {
  <<__DynamicallyCallable>> static function adder($a, $b) :mixed{return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b);}
  <<__DynamicallyCallable>> public function adder2($a, $b) :mixed{return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b);}
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reduce() : variation - object callbacks ***\n";

$array = vec[1];

echo "\n--- Static method callback ---\n";
var_dump(array_reduce($array, vec["A", "adder"]));

echo "\n--- Instance method callback ---\n";
var_dump(array_reduce($array, vec[new A(), "adder2"]));

echo "===DONE===\n";
}
