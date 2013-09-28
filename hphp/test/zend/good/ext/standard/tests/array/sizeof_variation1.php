<?php
/* Prototype  : int sizeof($mixed var[, int $mode])
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

echo "*** Testing sizeof() : usage variations ***\n";

echo "--- Testing sizeof() for all scalar types in default,COUNT_NORMAL and COUNT_RECURSIVE mode ---\n";
// get a resource variable
$fp = fopen(__FILE__, "r");

// array containing all scalar types 
$values = array (
           // int values
  /* 1  */  0,
            1,

            // float values
  /* 3  */   10.5,
            -10.5,
            12.3456789000e10,
            12.3456789000E-10,
  /* 7  */  .5,
            
            // NULL values
  /* 8  */  NULL,
            null,

            // boolean values 
  /* 10 */  TRUE,
            FALSE,
            true,
  /* 13 */  false,

            // string data 
  /* 14 */  "",
            '',  
            "string",
  /* 17 */  'string',

            // undefined variable 
            @$undefined_var,

            // resource variable 
  /* 19 */  $fp
);

// loop through the each value of the array for 'var' argument and check the behaviour of sizeof()
$counter = 1;
for($i = 0; $i < count($values); $i++)
{
  echo "-- Iteration $counter --\n";
 
  $var = $values[$i]; 

  echo "Default Mode: ";
  var_dump( sizeof($var) );
  echo "\n";

  echo "COUNT_NORMAL Mode: ";
  var_dump( sizeof($var, COUNT_NORMAL) );
  echo "\n";
     
  echo "COUNT_RECURSIVE Mode: ";
  var_dump( sizeof($var, COUNT_RECURSIVE) );
  echo "\n";
  
  $counter++;
}
      
echo "Done";
?>