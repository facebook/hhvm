<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $sort_flags argument to rsort() to test behaviour
 * Where possible, 'SORT_NUMERIC' has been entered as a string value
 */

echo "*** Testing rsort() : variation ***\n";

// Initialise function arguments not being substituted
$array_arg = array (1, 5, 2, 3, 1);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "SORT_NUMERIC";
  }
}

// heredoc string
$heredoc = <<<EOT
SORT_NUMERIC
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $sort_flags argument
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
/*18*/ "SORT_NUMERIC",
       'SORT_NUMERIC',
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
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  
  //create temporary array in case rsort() works
  $temp = $array_arg;
  
  var_dump( rsort($temp, $input) );
  var_dump($temp);
  $iterator++;
  
  $temp = null;
};

fclose($fp);

echo "Done";
?>
