<?php
/* Prototype  : proto int xml_set_object(resource parser, object &obj)
 * Description: Set up object which should be used for callbacks 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_set_object() : error conditions ***\n";


//Test xml_set_object with one more than the expected number of arguments
echo "\n-- Testing xml_set_object() function with more than expected no. of arguments --\n";

//WARNING: Unable to initialise parser of type resource

$obj = new stdclass();
$extra_arg = 10;
var_dump( xml_set_object(null, $obj, $extra_arg) );

// Testing xml_set_object with one less than the expected number of arguments
echo "\n-- Testing xml_set_object() function with less than expected no. of arguments --\n";

//WARNING: Unable to initialise parser of type resource

var_dump( xml_set_object(null) );

echo "Done";
?>
