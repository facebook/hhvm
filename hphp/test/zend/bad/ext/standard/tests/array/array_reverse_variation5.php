<?php
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_reverse() by giving associative arrays with different
 * values for $array argument
*/

echo "*** Testing array_reverse() : usage variations ***\n";

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

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// initializing the array
$arrays = array (

       // empty array
/*1*/  array(),

       // arrays with integer values
       array('0' => 0),
       array("1" => 1),
       array("one" => 1, 'two' => 2, "three" => 3, 4 => 4),
 
       // arrays with float values
/*5*/  array("float" => 2.3333),
       array("f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 33333333.333333),
  
       // arrays with string values
       array(111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3.3 =>  "pen\n"),
/*8*/  array(111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3.3 =>  'pen\n'), 
       array(1 => "hello", "heredoc" => $heredoc),

       // array with object, unset variable and resource variable
       array(11 => new classA(), "unset" => @$unset_var, "resource" => $fp), 

       // array with mixed values
/*11*/ array(1 => 'hello', 2 => new classA(), 222 => "fruit", 'resource' => $fp, "int" => 133, "float" => 444.432, "unset" => @$unset_var, "heredoc" => $heredoc)
);

// loop through the various elements of $arrays to test array_reverse()
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";
  // with default argument
  echo "- default argument -\n";
  var_dump( array_reverse($array) );
  // with $preserve_keys argument
  echo "- \$preserve keys = true -\n";
  var_dump( array_reverse($array, true) );
  echo "- \$preserve_keys = false -\n";
  var_dump( array_reverse($array, false) );
  $iterator++;
};

// close the file resource used
fclose($fp);

echo "Done";
?>