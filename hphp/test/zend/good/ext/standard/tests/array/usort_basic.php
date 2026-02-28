<?hh
/* Prototype  : bool usort(&array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of usort() with indexed and associative arrays
 */

function cmp($value1, $value2)
:mixed{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else
    return -1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing usort() : basic functionality ***\n";

// Int array with default keys
$int_values = vec[1, 8, 9, 3, 2, 6, 7];

echo "\n-- Numeric array with default keys --\n";
var_dump( usort(inout $int_values, cmp<>) );
var_dump($int_values);

// String array with default keys
$string_values = vec["This", "is", 'a', "test"];

echo "\n-- String array with default keys --\n";
var_dump( usort(inout $string_values, cmp<>) );
var_dump($string_values);

// Associative array with numeric keys
$numeric_key_arg = dict[1=> 1, 2 => 2, 3 => 7, 5 => 4, 4 => 9];

echo "\n-- Associative array with numeric keys --\n";
var_dump( usort(inout $numeric_key_arg, cmp<>) );
var_dump($numeric_key_arg);

// Associative array with string keys
$string_key_arg = dict['one' => 4, 'two' => 2, 'three' => 1, 'four' => 10];

echo "\n-- Associative array with string keys --\n";
var_dump( usort(inout $string_key_arg, cmp<>) );
var_dump($string_key_arg);
echo "===DONE===\n";
}
