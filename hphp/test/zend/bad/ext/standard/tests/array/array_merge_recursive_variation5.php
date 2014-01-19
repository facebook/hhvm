<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing different
 * associative arrays having different values to $arr1 argument.
*/

echo "*** Testing array_merge_recursive() : assoc. array with diff. values to \$arr1 argument ***\n";

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a resource variable
$fp = fopen(__FILE__, "r");

// get a class
class classA
{
  public function __toString(){
    return "Class A object";
  }
}

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different associative arrays to be passed to $arr1 argument
$arrays = array (
// arrays with integer values
/*1*/  array('0' => 0, '1' => 0),
       array("one" => 1, 'two' => 2, "three" => 1, 4 => 1),

       // arrays with float values
/*3*/  array("f1" => 2.3333, "f2" => 2.3333, "f3" => array(1.1, 2.22)),
       array("f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => array(1.2, 'f4' => 1.2)),

       // arrays with string values
/*5*/  array(111 => "\tHello", "array" => "col\tor", 2 => "\v\fworld", 3.3 =>  "\tHello"),
       array(111 => '\tHello', 'array' => 'col\tor', 2 => '\v\fworld', 3.3 =>  '\tHello'),
       array(1 => "hello", "string" => $heredoc, $heredoc),

       // array with object, unset variable and resource variable
/*8*/  array(11 => new classA(), "string" => @$unset_var, "resource" => $fp, new classA(), $fp),
);

// initialise the second array 
$arr2 = array( 1 => "one", 2, "string" => "hello", "array" => array("a", "b", "c"));

// loop through each sub array of $arrays and check the behavior of array_merge_recursive()
$iterator = 1;
foreach($arrays as $arr1) {
  echo "-- Iteration $iterator --\n";

  // with default argument
  echo "-- With default argument --\n";
  var_dump( array_merge_recursive($arr1) );

  // with more arguments
  echo "-- With more arguments --\n";
  var_dump( array_merge_recursive($arr1, $arr2) );

  $iterator++;
}

// close the file resource used
fclose($fp);
  
echo "Done";
?>