<?php
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association 
 * Source code: ext/standard/array.c
*/

/*
* Testing uasort() function by passing different scalar/nonscalar values as 'array_arg' argument
*/

echo "*** Testing uasort() : unexpected values for 'array_arg' ***\n";

// Comparison function
/* Prototype : int cmp_function(mixed $value1, mixed $value2)
 * Parameters : $value1 and $value2 - values to be compared
 * Return value : 0 - if both values are same
 *                1 - if value1 is greater than value2
 *               -1 - if value1 is less than value2
 * Description : compares value1 and value2
 */
function cmp_function($value1, $value2)
{
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

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get resource variable
$fp = fopen(__FILE__,'r');

//array of values to iterate over
$input_values = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       10.1234567e8,
       10.7654321E-8,
       .5,

       // null data
/*10*/ NULL,
       null,
      
       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',

       // resource data
/*20*/ $fp,

       // undefined data
       @$undefined_var,
 
       // unset data
/*22*/ @$unset_var,
);

// loop through each value of input_values
for($count = 0; $count < count($input_values); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( uasort($input_values[$count], 'cmp_function') );
};

//closing resource
fclose($fp);
echo "Done"
?>