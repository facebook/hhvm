<?php

echo "\n*** Testing array_values() with resource type ***\n";
$resource1 = fopen(__FILE__, "r");  // Creating a file resource
$resource2 = opendir(".");  // Creating a dir resource

/* creating an array with resources as elements */
$arr_resource = array( "a" => $resource1, "b" => $resource2);
var_dump( array_values($arr_resource) );

echo "\n*** Testing array_values() with range checking ***\n";
$arr_range = array(
  2147483647, 
  2147483648, 
  -2147483647,
  -2147483648,
  -0,
  0,
  -2147483649
);
var_dump( array_values($arr_range) );

echo "\n*** Testing array_values() on an array created on the fly ***\n";
var_dump( array_values(array(1,2,3)) );
var_dump( array_values(array()) );  // null array

echo "Done\n";
?>