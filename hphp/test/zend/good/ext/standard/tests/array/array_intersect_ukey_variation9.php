<?hh
/* Prototype  : array array_intersect_ukey(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays using a callback function on the keys for comparison.
 * Source code: ext/standard/array.c
 */

class MyClass
{
  <<__DynamicallyCallable>> static function static_compare_func($key1, $key2) :mixed{
    return strcasecmp($key1, $key2);
  }

  <<__DynamicallyCallable>> public function class_compare_func($key1, $key2) :mixed{
    return strcasecmp($key1, $key2);
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_ukey() : usage variation ***\n";

//Initialise arguments
$array1 = dict['blue'  => 1, 'red'  => 2, 'green'  => 3, 'purple' => 4];
$array2 = dict['green' => 5, 'blue' => 6, 'yellow' => 7, 'cyan'   => 8];

echo "\n-- Testing array_intersect_ukey() function using class with static method as callback --\n";
var_dump( array_intersect_ukey($array1, $array2, vec['MyClass','static_compare_func']) );
var_dump( array_intersect_ukey($array1, $array2, 'MyClass::static_compare_func') );

echo "\n-- Testing array_intersect_uassoc() function using class with regular method as callback --\n";
$obj = new MyClass();
var_dump( array_intersect_ukey($array1, $array2, vec[$obj,'class_compare_func']) );
echo "===DONE===\n";
}
