<?hh
/* Prototype  : bool usort(&array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array with duplicate keys and values to usort() to test behaviour
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
echo "*** Testing usort() : usage variation ***\n";

// Array with duplicate string and integer keys and values
$array_arg = dict[0 => 2,     "a" => 8, "d" => 9,
                   3 => 3,     5 => 2,   "o" => 6,
                   "z" => -99, 0 => 1,   "z" => 3];

echo "\n-- Array with duplicate keys --\n";
var_dump( usort(inout $array_arg, cmp<>) );
var_dump($array_arg);

// Array with default and assigned keys
$array_arg = dict[0 => "Banana", 1 => "Mango", 2 => "Orange", 2 => "Apple", 3 => "Pineapple"];

echo "\n-- Array with default/assigned keys --\n";
var_dump( usort(inout $array_arg, cmp<>) );
var_dump($array_arg);
echo "===DONE===\n";
}
