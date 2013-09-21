<?php
/* Prototype  : int sizeof($mixed var[, int $mode])
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

echo "*** Testing sizeof() : usage variations ***\n";

echo "--- Testing sizeof() with different values for 'mode' argument ---\n";
$array1 = array(1, 2, 3, 4, array(1.0, 2.0, array()), array() );

// get a resource variable
$fp = fopen(__FILE__, "r");

//unset variable
$unset_var = 10;
unset($unset_var);

//class declaration
class test
{
  public $member1;
}

$mode_values = array (
  /* 1  */  COUNT_NORMAL,
            COUNT_RECURSIVE,
            0,  // same as COUNT_NORMAL
            1,  // same as COUNT_RECURSIVE

  /* 5  */  TRUE,  // same as COUNT_RECURSIVE
            true,  // same as COUNT_RECURSIVE
            FALSE, // same as COUNT_NORMAL
            false, // same as COUNT_NORMAL
            NULL,  // same as COUNT_NORMAL
  /* 10 */  null,  // same as COUNT_NORMAL
            100,
            10.5,
            12.34e3,
            12.34E-2,
  /* 15 */  .5,
            "",
            '',
            "string",
            'string',
  /* 20 */  @$unset_var,
            new test(),
  /* 22 */  $fp
);
  
// loop through the each element of $modes_array for 'mode' argument 
// and check the working of sizeof()
$counter = 1;
for($i = 0; $i < count($mode_values); $i++)
{
  echo "-- Iteration $counter --\n";
  $mode = $mode_values[$i];
  
  var_dump( sizeof($array1, $mode) );

  $counter++;
}

fclose($fp);

echo "Done";
?>