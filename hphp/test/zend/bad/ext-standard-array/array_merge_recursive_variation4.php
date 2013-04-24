<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing different
 * associative arrays having different keys to $arr1 argument.
*/

echo "*** Testing array_merge_recursive() : assoc. array with diff. keys to \$arr1 argument ***\n";

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
/*1*/  // arrays with integer keys
       array(0 => "0", 1 => array(1 => "one")),
       array(1 => "1", 2 => array(1 => "one", 2 => "two", 3 => 1, 4 => "4")),

       // arrays with float keys
/*3*/  array(2.3333 => "float", 44.44 => array(1.1 => "float")),
       array(1.2 => "f1", 3.33 => "f2", 4.89999922839999 => array(1.1 => "f1"), 3333333.333333 => "f4"),

       // arrays with string keys
/*5*/  array('\tHello' => array("hello", 'world'), '\v\fworld' => 2.2, 'pen\n' => 111),
       array("\tHello" => array("hello", 'world'), "\v\fworld" => 2.2, "pen\n" => 111),
       array("hello", $heredoc => array("heredoc", 'string'), "string"),

       // array with object, unset variable and resource variable
/*8*/ array(new classA() => 11, @$unset_var => array("unset"), $fp => 'resource', 11, "hello")
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