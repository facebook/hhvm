<?hh
/* Prototype  : array array_intersect_assoc(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments.
 * Keys are used to do more restrictive check
 * Source code: ext/standard/array.c
*/

/*
* Testing the behavior of array_intersect_assoc() by passing 2-D arrays
* to both $arr1 and $arr2 argument.
* Optional argument takes the same value as that of $arr1
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_assoc() : passing two dimensional array to both \$arr1 and \$arr2 arguments ***\n";

// two dimensional arrays for $arr1 and $arr2 argument
$arr1 = varray [

  // arrays with default keys
  vec[1, 2, "hello", 'world'],
  vec[1, 2, 3, 4],

  // arrays with explicit keys
  dict[1 => "one", 2 => "two", 3 => "three"],
  dict["ten" => 10, "twenty" => 20.00, "thirty" => 30]
];

$arr2 = varray [
  vec[1, 2, 3, 4],
  dict[1 => "one", 2 => "two", 3 => "three"]
];

/* Passing the sub-array as argument to $arr1 and $arr2 */
// Calling array_intersect_assoc() with default arguments
echo "-- Passing the sub-array to \$arr1 and \$arr2 --\n";
echo "- With default arguments -\n";
var_dump( array_intersect_assoc($arr1[0], $arr2[0]) );

// Calling array_intersect_assoc() with more arguments
// additional argument passed is the same as $arr1
echo "- With more arguments -\n";
var_dump( array_intersect_assoc($arr1[0], $arr2[0], $arr1[0]) );

echo "Done";
}
