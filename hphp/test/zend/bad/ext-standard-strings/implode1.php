<?php
/* Prototype: string implode ( string $glue, array $pieces );
   Description: Returns a string containing a string representation of all the 
                array elements in the same order, with the glue string between each element.
*/
echo "*** Testing implode() for basic opeartions ***\n";
$arrays = array (
  array(1,2),
  array(1.1,2.2),
  array(array(2),array(1)),
  array(false,true),
  array(),
  array("a","aaaa","b","bbbb","c","ccccccccccccccccccccc")
);
/* loop to output string with ', ' as $glue, using implode() */
foreach ($arrays as $array) {
  var_dump( implode(', ', $array) );
  var_dump($array);
}

echo "\n*** Testing implode() with variations of glue ***\n";
/* checking possible variations */
$pieces = array (
  2, 
  0,
  -639,
  true,
  "PHP",
  false,
  NULL,
  "",
  " ",
  "string\x00with\x00...\0"
);
$glues = array (
  "TRUE",
  true,
  false,
  array("key1", "key2"),
  "",
  " ",
  "string\x00between",
  NULL, 
  -0,
  '\0'
);
/* loop through to display a string containing all the array $pieces in the same order,
   with the $glue string between each element  */
$counter = 1;
foreach($glues as $glue) {
  echo "-- Iteration $counter --\n";
  var_dump( implode($glue, $pieces) );
  $counter++;
}

/* empty string */
echo "\n*** Testing implode() on empty string ***\n";
var_dump( implode("") );

/* checking sub-arrays */
echo "\n*** Testing implode() on sub-arrays ***\n";
$sub_array = array(array(1,2,3,4), array(1 => "one", 2 => "two"), "PHP", 50);
var_dump( implode("TEST", $sub_array) );
var_dump( implode(array(1, 2, 3, 4), $sub_array) );
var_dump( implode(2, $sub_array) );

echo "\n*** Testing implode() on objects ***\n";
/* checking on objects */
class foo
{
  function __toString() {
    return "Object";
  }
}

$obj = new foo(); //creating new object
$arr = array();
$arr[0] = &$obj;
$arr[1] = &$obj;
var_dump( implode(",", $arr) );
var_dump($arr);

/* Checking on resource types */
echo "\n*** Testing end() on resource type ***\n";
/* file type resource */
$file_handle = fopen(__FILE__, "r");

/* directory type resource */
$dir_handle = opendir( dirname(__FILE__) );

/* store resources in array for comparison */
$resources = array($file_handle, $dir_handle);

var_dump( implode("::", $resources) );

echo "\n*** Testing error conditions ***\n";
/* zero argument */
var_dump( implode() );

/* only glue */
var_dump( implode("glue") );

/* int as pieces */
var_dump( implode("glue",1234) );

/* NULL as pieces */
var_dump( implode("glue", NULL) );

/* pieces as NULL array */
var_dump( implode(",", array(NULL)) );

/* integer as glue */
var_dump( implode(12, "pieces") );

/* NULL as glue */
var_dump( implode(NULL, "abcd") );

/* args > than expected */
var_dump( implode("glue", "pieces", "extra") );

/* closing resource handles */
fclose($file_handle);
closedir($dir_handle);

echo "Done\n";
?>