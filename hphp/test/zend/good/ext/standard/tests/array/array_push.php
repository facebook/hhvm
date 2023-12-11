<?hh

/* Prototype: int array_push(& array &array );
 * Description: Push one or more elements onto the end of array
 and returns the new number of elements in the array.
 */
<<__EntryPoint>> function main(): void {
$empty_array = vec[];
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array1 = vec[ 1,2,3,4,5,6,7,8,9 ];
$mixed_array2 = vec[ "One", "_Two", "Three", "Four", "Five" ];
$mixed_array = vec[
  vec[],
  $mixed_array1,
  $mixed_array2,
  vec[ 6, "six", 7, "seven", 8, "eight", 9, "nine" ],
  dict[ "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee" ],
  dict[ "1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five" ],
  dict[ 1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five" ],
  dict[ "f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2 => "float", "F" => "FFF",
         "blank" => "", 3 => 3.7, 5 => 7, 6 => 8.6, '5' => "Five", "4name" => "jonny", "a" => NULL, '' => 3 ],
  vec[ 12, "name", 'age', '45' ],
  vec[ vec["oNe", "tWo", 4], vec[10, 20, 30, 40, 50], vec[] ],
  dict[ "one" => 1, "one" => 2, "three" => 3, 0 => 3, 1 => 4, 3 => 33, 4 => 44, 5 => 5, 6 => 6,
          5 => 54, 5 => 57, "5.4" => 554, "5.7" => 557 ]
];

/* Error Conditions */
echo "\n*** Testing Error Conditions ***\n";

/* Zero argument  */
try { var_dump( array_push() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Scalar argument */
var_dump( array_push(inout $number, 22) );

/* String argument */
var_dump( array_push(inout $str, 22) );

/* Invalid Number of arguments */
var_dump( array_push(inout $mixed_array1, 1,2) );

/* Empty Array as argument */
var_dump( array_push(inout $empty_array, 2) );

$mixed_array[1] = $mixed_array1;
$mixed_array[2] = $mixed_array2;

/* Loop to test normal functionality with different arrays inputs */
echo "\n*** Testing with various array inputs ***\n";

$counter = 1;
foreach( $mixed_array as $sub_array )
{
 echo "\n-- Input Array for Iteration $counter is --\n";
 print_r( $sub_array );
 echo "\nOutput after push is :\n";
 var_dump( array_push(inout $sub_array, 22, "abc") );
 $counter++;
}

/* Checking for return value and the new array formed from push operation */
echo "\n*** Checking for return value and the new array formed from push operation ***\n";
var_dump( array_push(inout $mixed_array2, 22, 33, "44") );
var_dump( $mixed_array2 );

echo"\nDone";
}
