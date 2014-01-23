<?php
/* Prototype  : proto string base64_decode(string str[, bool strict])
 * Description: Decodes string using MIME base64 algorithm 
 * Source code: ext/standard/base64.c
 * Alias to functions: 
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
set_error_handler('test_error_handler');

echo "*** Testing base64_decode() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$strict = true;

//getting the resource
$file_handle = fopen(__FILE__, "r");

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values =  array (
   	// int data
	"0" =>  0,
	"1" =>  1,
	"12345" =>  12345,
	"-2345"	=>  -2345,
		
	// float data
    "10.5" =>  10.5,
	"-10.5" => -10.5,
	"10.1234567e10" =>	10.1234567e10,
	"10.7654321E-10" => 10.7654321E-10,
	".5" => .5,
		
    // array data
    "array()" =>   array(),
	"array(0)" =>  array(0),
	"array(1)" =>  array(1),
	"array(1, 2)" => array(1, 2),
	"array('color' => 'red', 'item' => 'pen'" => array('color' => 'red', 'item' => 'pen'),
		
	// null data
	"NULL" => NULL,
	"null" => null,
		
	// boolean data
	"true" => true,
	"false" => false,
	"TRUE" => TRUE,
	"FALSE" => FALSE,
		
	// empty data
	"\"\"" => "",
	"''" => '',
		
	// object data
	"stdClass object" => new stdclass(),
		
	// undefined data
    "undefined variable" => $undefined_var,
		
	// unset data
	"unset variable" => $unset_var,
	
	// resource data
	"resource" => $file_handle
);

// loop through each element of the array for str argument

foreach($values as $key=>$value) {
    echo "\n-- Arg value $key --\n";
    $output =  base64_decode($value, $strict);
       
	if (is_string($output)) { 
		var_dump(bin2hex($output));
    } else {
        var_dump($output); 
    }	
};

?>
===Done===