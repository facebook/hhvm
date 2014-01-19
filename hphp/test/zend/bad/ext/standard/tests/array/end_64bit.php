<?php
/* Prototype: mixed end ( array &$array );
   Description: Advances internal pointer of array to last element, and returns its value.
*/

$arrays = array (
  array( 0 ),
  range(1, 100 ),
  range('a', 'z', 2 ),
  array("a" => "A", 2 => "B", "C" => 3, 4 => 4, "one" => 1, "" => NULL ),
  array(1, array(1, 2 => 3 ), "one" => 1, "5" => 5 ),
  array(-1, -2, -3, -4, "-0.005" => "neg0.005", 2.0 => "float2", "neg.9" => -.9 ),
  array(1.0005, 2.000000, -3.000000, -4.9999999 ),
  array(true, false),
  array("PHP", "Web2.0", "SOA"),
  array(1, array() ),
  array(1, 2, "" ),
  array(" "),
  array(2147483647, 2147483648, -2147483647, -2147483648 ),
  array(0x7FFFFFFF, -0x80000000, 017777777777, -020000000000 ),
  array(-.6700000E+3, -4.10003E+3, 1e-5, -1E+5, 000002.00 )
);
/* loop through $arrays to print the last element of each sub-array */ 
echo "*** Testing end() on different arrays ***\n";
$counter = 1;
foreach ($arrays as $sub_array){
  echo "-- Iteration $counter --\n";
  var_dump( end($sub_array) );
  /* ensure that internal pointer is moved to last element */
  var_dump( current($sub_array) ); 
  $counter++;
}

/* checking for end() on sub-arrays */
echo "\n*** Testing end() with sub-arrays ***\n";
$test_array = array(1, array(1 => "one", "two" => 2, "" => "f") );
var_dump( end($test_array) );
var_dump( end($test_array[1]) );

/* checking working of end() when array elements are deleted */
echo "\n*** Testing end() when array elements are deleted ***\n";
$array_test = array("a", "b", "d", 7, "u" => "U", -4, "-.008" => "neg.008");

// remove first element from array 
echo "\n-- Remove first element from array --\n";
unset($array_test[0]);
var_dump( end($array_test) );

// remove last element from array, rewind and check end() 
echo "\n-- Remove last element from array --\n";
unset($array_test['-.008']);
var_dump( end($array_test) );
reset( $array_test );
var_dump( end($array_test) );

// remove any element  !first, !last, rewind and check end() 
echo "\n-- Remove any element from array apart from first and last element --\n";
unset($array_test[7]);
var_dump( end($array_test) );
var_dump( reset($array_test) );
var_dump( end($array_test) );

/* Checking on OBJECTS type */
echo "\n*** Testing end() on objects ***\n";
class foo
{
  function __toString() {
    return "Object";
  }
}
class foo1
{
  function __toString() {
    return "Object1";
  }
}

$object1 = new foo(); //new object created
$object2 = new foo1();

$array_object = array();
$array_object[0] = &$object1;
$array_object[1] = &$object2;
var_dump( end($array_object) );
var_dump($array_object);

/* Checking on RESOURCE type */
echo "\n*** Testing end() on resource type ***\n";
//file type resource
$file_handle = fopen(__FILE__, "r");

//directory type resource
$dir_handle = opendir( dirname(__FILE__) );

//store resources in array
$resources = array($file_handle, $dir_handle);
var_dump( end($resources) );
var_dump( current($resources) );

echo "\n*** Testing error conditions ***\n";
/* checking for unexpected number of arguments */
var_dump( end() );
var_dump( end($array[0], $array[0]) );

/* checking for unexpected type of arguments */
$var=1;
$var1="string";
var_dump( end($var) );
var_dump( end($var1) );

/* checking null array */
$null_array = array();
var_dump( end($null_array) );

echo "Done\n";

?>
/* cleaning resource handles */
fclose( $file_handle );  //file resource handle deleted
closedir( $dir_handle );  //dir resource handle deleted
