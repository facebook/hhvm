<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing associative array with different keys for $arr1 argument
 */

echo "*** Testing array_map() : associative array with diff. keys for 'arr1' argument ***\n";

function callback($a)
{
  return ($a);
}

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a resource variable
$fp = fopen(__FILE__, "r");

// get a class
class classA{
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

       // arrays with integer keys
/*2*/  array(0 => "0"),
       array(1 => "1"),
       array(1 => "1", 2 => "2", 3 => "3", 4 => "4"),

       // arrays with float keys
/*5*/  array(2.3333 => "float"),
       array(1.2 => "f1", 3.33 => "f2", 4.89999922839999 => "f3", 33333333.333333 => "f4"),

       // arrays with string keys
       array('\tHello' => 111, 're\td' => 'color', '\v\fworld' => 2.2, 'pen\n' => 33),
/*8*/  array("\tHello" => 111, "re\td" => "color", "\v\fworld" => 2.2, "pen\n" => 33),
       array("hello", $heredoc => "string"), // heredoc

       // array with object, unset variable and resource variable
       array(new classA() => 11, @$unset_var => "hello", $fp => 'resource'),

       // array with mixed values
/*11*/ array('hello' => 1, new classA() => 2, "fruit" => 2.2, 
              $fp => 'resource', 133 => "int", 444.432 => "float", 
              @$unset_var => "unset", $heredoc => "heredoc")
);

// loop through the various elements of $arrays to test array_map()
$iterator = 1;
foreach($arrays as $arr1) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_map('callback', $arr1) );
  $iterator++;
}

echo "Done";
?>