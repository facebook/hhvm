<?hh
/* Prototype  : proto array array_fill_keys(array keys, mixed val)
 * Description: Create an array using the elements of the first parameter as keys each initialized to val
 * Source code: ext/standard/array.c
 * Alias to functions:
 */


<<__EntryPoint>>
function main() {
  echo "*** Testing array_fill_keys() : parameter variations ***\n";

  $nullVal = null;
  $simpleStr = "simple";
  $emptyArr = varray[];

  echo "\n-- Testing array_fill_keys() function with empty arguments --\n";
  var_dump( array_fill_keys($emptyArr, $nullVal) );

  echo "\n-- Testing array_fill_keys() function with keyed array --\n";
  $keyedArray = darray["two" => 2, "strk1" => "strv1", 0 => 4, 1 => $simpleStr];
  var_dump( array_fill_keys($keyedArray, $simpleStr) );

  echo "Done";
}
