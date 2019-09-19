<?hh
/* Prototype: mixed array_shift(& array &array );
 * Description: Shifts the first value of the array off and returns it.
 */
<<__EntryPoint>> function main(): void {
$empty_array = array();
$number = 5;
$str = "abc";


/* Various combinations of arrays to be used for the test */
$mixed_array1 = array( 1,2,3,4,5,6,7,8,9 );
$mixed_array = array(
  array(),
  $mixed_array1,
  array( "One", "_Two", "Three", "Four", "Five" ),
  array( 6, "six", 7, "seven", 8, "eight", 9, "nine" ),
  array( "a" => "aaa", "A" => "AAA", "c" => "ccc", "d" => "ddd", "e" => "eee" ),
  array( "1" => "one", "2" => "two", "3" => "three", "4" => "four", "5" => "five" ),
  array( 1 => "one", 2 => "two", 3 => 7, 4 => "four", 5 => "five" ),
  array( "f" => "fff", "1" => "one", 4 => 6, "" => "blank", 2.4 => "float", "F" => "FFF",
         "blank" => "", 3.7 => 3.7, 5.4 => 7, 6 => 8.6, '5' => "Five", "4name" => "jonny", "a" => NULL, NULL => 3 ),
  array( 12, "name", 'age', '45' ),
  array( array("oNe", "tWo", 4), array(10, 20, 30, 40, 50), array() ),
  array( "one" => 1, "one" => 2, "three" => 3, 3, 4, 3 => 33, 4 => 44, 5, 6,
                              5.4 => 54, 5.7 => 57, "5.4" => 554, "5.7" => 557 )
);

/* Testing Error Conditions */
echo "\n*** Testing Error Conditions ***\n";

/* Zero argument  */
try { var_dump( array_shift() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Scalar argument */
var_dump( array_shift(&$number) );

/* String argument */
var_dump( array_shift(&$str) );

/* Invalid Number of arguments */
try { var_dump( array_shift(&$mixed_array1,$mixed_array[2]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Empty Array as argument */
var_dump( array_shift(&$empty_array) );

/* Loop to test normal functionality with different arrays inputs */
echo "\n*** Testing with various array inputs ***\n";

$counter = 1;
foreach( $mixed_array as $sub_array ) {
  echo "\n-- Input Array for Iteration $counter is --\n";
  print_r( $sub_array );
  echo "\nOutput after shift is :\n";
  var_dump( array_shift(&$sub_array) );
  $counter++;
}

/*Checking for internal array pointer beint reset when shift is called */

echo"\n*** Checking for internal array pointer being reset when shift is called ***\n";

echo "\nCurrent Element is : ";
var_dump( current($mixed_array1) );

echo "\nNext Element is : ";
var_dump( next(inout $mixed_array1) );

echo "\nNext Element is : ";
var_dump( next(inout $mixed_array1) );

echo "\nshifted Element is : ";
var_dump( array_shift(&$mixed_array1) );

echo "\nCurrent Element after shift operation is: ";
var_dump( current($mixed_array1) );

echo"Done";
}
