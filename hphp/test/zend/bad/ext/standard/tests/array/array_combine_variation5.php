<?php
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys
 *              and the elements of the second as the corresponding values
 * Source code: ext/standard/array.c
*/

/*
* Testing the functionality of array_combine() by passing various
* associative arrays having different possible values to $keys argument and
* associative arrays having different possible values to $values argument.
*/

echo "*** Testing array_combine() : assoc array with diff values to both \$keys and \$values argument ***\n";

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

// different variations of associative array
$arrays = array (

       // empty array
/*1*/  array(),

       // arrays with integer values
       array('0' => 0),
       array("1" => 1),
       array("one" => 1, 'two' => 2, "three" => 3, 4 => 4),

       // arrays with float values
/*5*/  array("float" => 2.3333),
       array("f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 33333333.333),

       // arrays with string values
/*7*/  array(111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3.3 =>  "pen\n"),
       array(111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3.3 =>  'pen\n'),
       array(1 => "hello", "heredoc" => $heredoc),

       // array with object, unset variable and resource variable
/*10*/ array(11 => new classA(), "unset" => @$unset_var, "resource" => $fp),

       // array with mixed values
/*11*/ array(1 => 'hello', 2 => new classA(), 222 => "fruit", 
             'resource' => $fp, "int" => 133, "float" => 444.432, 
             "unset" => @$unset_var, "heredoc" => $heredoc)
);


// loop through each sub-array within $arrays to check the behavior of array_combine()
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_combine($array, $array) );
  $iterator++;
}

// close the file resource used
fclose($fp);

echo "Done";
?>