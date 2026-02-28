<?hh
/* Prototype  : bool usort(&array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array of referenced variables as $array_arg to test behaviour
 */

function cmp_function($value1, $value2)
:mixed{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else {
    return -1;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing usort() : usage variation ***\n";

// different variables which are used as elements of $array_arg
$value1 = -5;
$value2 = 100;
$value3 = 0;


$array_arg = dict[
  0 => 10,
  1 => $value1,
  2 => $value2,
  3 => 200,
  4 => $value3,
];

echo "\n-- Sorting \$array_arg containing different references --\n";
var_dump( usort(inout $array_arg, cmp_function<>) );
var_dump($array_arg);
echo "===DONE===\n";
}
