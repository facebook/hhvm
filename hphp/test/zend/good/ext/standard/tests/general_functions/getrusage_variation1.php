<?php
/* Prototype  :  array getrusage  ([ int $who  ] )
 * Description: Gets the current resource usages
 * Source code: ext/standard/microtime.c
 * Alias to functions: 
 */


/*
 * Pass different data types as $who argument to test behaviour of getrusage()
 */

echo "*** Testing getrusage() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);


// unexpected values to be passed to $stream_id argument
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
       
       // string data
/*16*/ "0",
       '1',
       "1232456",
       "1.23E4",
       
       // undefined data
/*20*/ @$undefined_var,

       // unset data
/*21*/ @$unset_var,
);

// loop through each element of $inputs to check the behavior of getrusage()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  $res = getrusage($input);
  echo "User time used (microseconds) " . $res["ru_utime.tv_usec"] . "\n";
  $iterator++;
}
?>
===DONE===