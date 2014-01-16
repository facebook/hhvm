<?php
/* Prototype: array get_defined_vars(void);
 * Description: Returns a  multidimentional array of all defined variables.
 */

/* Various variables definitions used for testing of the function */

$number = 22.33; //number
$string = "sample string"; //string
$array1 = array(1, 1, 2, 3, 5, 8); //simple array
$assoc_array = array( 'a'=>97, 'c'=>99, 'A'=>65, 'C'=>67, 1=>"string1" ); //associative array
$boolean = TRUE; //boolean

/* Checking for Class and Objects */
class sample {
var $number = 233;
var $string = "string2";
public function func() {
$local_var = 2;
var_dump( get_defined_vars() );
}
}
$sample_obj = new sample; //object declaration

function func() {
$string33 = 22;
var_dump( get_defined_vars() );
}

$arr = get_defined_vars();

/* Displaying various variable through the array captured by the get_defined_vars function call */
echo "\n*** Displaying various variables through the array captured by the get_defined_vars function call ***\n";
var_dump( $arr["argc"] ); 
var_dump( $arr["number"] );
var_dump( $arr["string"] );
var_dump( $arr["array1"] );
var_dump( $arr["assoc_array"] );
var_dump( $arr["boolean"] );
var_dump( $arr["sample_obj"] );


echo "\n*** Checking for output when get_defined_vars called in local function ***\n";
func();


echo "\n*** Checking for output when get_defined_vars called in function of a class ***\n";
$sample_obj->func();

echo "\n*** Checking for output when get_defined_vars called in nested functions ***\n";
function func1(){
$func1_var = 2;
var_dump( get_defined_vars() );
function func2(){
$func2_var = 3;
var_dump( get_defined_vars() );
}
func2();
}
func1();

echo "\nDone";
?> 