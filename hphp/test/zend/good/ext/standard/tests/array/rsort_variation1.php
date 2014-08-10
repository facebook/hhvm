<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $array_arg argument to rsort() to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $array_arg argument
$inputs = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
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
       $heredoc,
       
       // object data
/*21*/ new classA(),

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,

       // resource variable
/*24*/ $fp
);

// loop through each element of $inputs to check the behavior of rsort()
$iterator = 1;
foreach ($inputs as $input) {
  echo "-- Iteration $iterator --\n";
  echo "Flag = default:\n";
  var_dump( rsort($input) );
  echo "Flag = SORT_REGULAR:\n";
  var_dump( rsort($input, SORT_REGULAR) );
  echo "Flag = SORT_NUMERIC:\n";
  var_dump( rsort($input, SORT_NUMERIC) );
  echo "Flag = SORT_STRING:\n";
  var_dump( rsort($input, SORT_STRING) );
  $iterator++;
}

fclose($fp);

echo "Done";
?>

