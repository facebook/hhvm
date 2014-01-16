<?php
/* Prototype  : int sizeof($mixed var[, int $mode])
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

echo "*** Testing sizeof() : usage variations ***\n";

echo "--- Testing sizeof() for all kinds of unset variables in default, Normal and Recursive Modes ---\n";

// class declaration
class test
{  
  public $member1;
}

// get an resource variable 
$fp = fopen(__FILE__, "r");

// array containing different types of variables
$values = array (
            // int values
  /* 1  */  0,
            1,
            // float values
  /* 3  */  10.5,
            -10.5,
            12.34e3,
  /* 6  */  12.34E-3,
            // string values
  /* 7  */  "string",
            'string',
            "",
  /* 10 */  '',
            // NULL values
  /* 11 */  NULL,
            null,
            // Boolean Values
  /* 12 */  TRUE,
            true,
            false,
  /* 16 */  FALSE,
            // array values
  /* 17 */  array(),
            array(1, 2, 3,4 , array(5, 6)),
            // object variable
  /* 19 */  new test(),
            // resource variable
  /* 20 */  $fp
);

// loop through the each element of the $values array for 'var' argument 
// and check the functionality of sizeof()
$counter = 1;
foreach($values as $value)
{
  echo "-- Iteration $counter --\n";
 
  // unset the variable 
  unset($value);

  // now check the size of unset variable when different modes are given
  echo "Default Mode: ";
  var_dump( sizeof($value) );
  echo "\n";
 
  echo "COUNT_NORMAL Mode: ";
  var_dump( sizeof($value, COUNT_NORMAL) );
  echo "\n";

  echo "COUNT_RECURSIVE Mode: ";
  var_dump( sizeof($value, COUNT_RECURSIVE) );
  echo "\n";

  $counter++;
}

fclose($fp);

echo "Done";
?>