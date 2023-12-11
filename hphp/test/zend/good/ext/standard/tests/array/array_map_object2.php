<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Testing array_map() for following object functionalities:
 *   1) non-existent class
 *   2) existent class and non-existent function
 */
class SimpleClass
{
  public $var1 = 1;
  public function square($n) :mixed{
    return $n * $n;
  }
  public static function cube($n) :mixed{
    return $n * $n * $n;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() :  with non-existent class and method ***\n";

echo "-- with non-existent class --\n";
var_dump( array_map(vec['non-existent', 'square'], vec[1, 2]) );

echo "-- with existent class and non-existent method --\n";
var_dump( array_map(vec['SimpleClass', 'non-existent'], vec[1, 2]) );

echo "Done";
}
