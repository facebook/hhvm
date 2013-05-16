<?php
/* Prototype  : mixed array_sum(array $input)
 * Description: Returns the sum of the array entries 
 * Source code: ext/standard/array.c
*/

/*
* Passing different scalar/nonscalar values as 'input' argument to array_sum() 
*/

echo "*** Testing array_sum() : unexpected values for 'input' ***\n";

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// Class definition
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

// different scalar/non scalar values for 'input' argument
$input_values = array(

         // int data
/*1*/    0,
         1,
         12345,
         -2345,

         // float data
/*5*/    10.5,
         -10.5,
         10.1234567e8,
         10.7654321E-8,
         .5,

         // null data
/*10*/   NULL,
         null,

         // boolean data
/*12*/   true,
         false,
         TRUE,
         FALSE,

         // empty data
/*16*/   "",
         '',

         // string data
/*18*/   "string",
         'string',

         // object data
/*20*/   new MyClass(),

         // resource data
/*21*/   $fp = fopen(__FILE__,'r'),

         // undefined data
/*22*/   @$undefined_var,

         // unset data
/*23*/   @$unset_var,
);

// loop through each element of the array for input
for($count = 0; $count < count($input_values); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( array_sum($input_values[$count]) );
};

fclose($fp);
echo "Done"
?>