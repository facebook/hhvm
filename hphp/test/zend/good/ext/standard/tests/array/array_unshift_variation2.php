<?php
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_unshift() by giving all the possible values for $var argument
*/

echo "*** Testing array_unshift() : all possible values for \$var argument ***\n";

// array to be passed to $array argument
$array = array('f' => "first", "s" => 'second', 1, 2.222);

// get a class 
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// get a resource variable
$fp = fopen(__FILE__, "r");

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// different types of values to be passed to $var argument
$vars = array(

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

       // array data
/*10*/ array(),
       array(0),
       array(1),
       array(1, 2),
       array('color' => 'red', 'item' => 'pen'),
 
       // null data
/*15*/ NULL,
       null,

       // boolean data
/*17*/ true,
       false,
       TRUE,
       FALSE,
 
       // empty data
/*21*/ "",
       '',

       // string data
/*23*/ "string",
       'string',
       $heredoc,

       // object data
/*26*/ new classA(),

       // undefined data
       @$undefined_var,

       // unset data
       @$unset_var,
 
       // resource variable
/*29*/ $fp
);

// loop through each element of $vars to check the functionality of array_unshift()
$iterator = 1;
foreach($vars as $var) {
  echo "-- Iteration $iterator --\n";
  $temp_array = $array;

  /* with default argument */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  var_dump( array_unshift($temp_array, $var) );

  // dump the resulting array
  var_dump($temp_array);  

  /* with optional arguments */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift($temp_array, $var, "hello", 'world') );

  // dump the resulting array
  var_dump($temp_array);
  $iterator++;
}

// close the file resource used
fclose($fp);

echo "Done";
?>