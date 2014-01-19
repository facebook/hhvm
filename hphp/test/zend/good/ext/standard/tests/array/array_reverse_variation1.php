<?php
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_reverse() : usage variations - unexpected values for 'array' argument ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get a resource variable
$fp = fopen(__FILE__, "r");

//get a class
class classA
{
  public function __toString(){
    return "Class A object";
  }
}

//get a heredoc string
$heredoc_string = <<<EOT
Hello world\t\n
EOT;

//array of values to iterate over
$arrays = array (

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       10.5e10,
       10.6E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
       true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',
      
       // string data
       'Hello world',
       "Hello world",
       $heredoc_string,
  
       // object data
/*21*/ new classA(),

       // undefined data
       @$undefined_var,

       // unset data
       @$unset_var,

       // resource variable
/*24*/ $fp

);

// loop through each element of the array $arrays to check the behavior of array_reverse()
$iterator = 1;
foreach($arrays as $array) {
  echo "\n-- Iteration $iterator --";
  // with default argument
  var_dump( array_reverse($array) );
  // with all possible arguments
  var_dump( array_reverse($array, true) );
  var_dump( array_reverse($array, false) );
  $iterator++;
};

// close the file resource used
fclose($fp);

echo "Done";
?>