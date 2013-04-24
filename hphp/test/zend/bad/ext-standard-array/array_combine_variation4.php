<?php
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys
 *              and the elements of the second as the corresponding values
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_combine() by passing different
 * associative arrays having different possible keys to $keys argument and 
 * associative arrays having different possible keys to $values argument.
*/

echo "*** Testing array_combine() : assoc array with diff keys to both \$keys and \$values argument ***\n";
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

// different variations of associative arrays to be passed to $arr1 argument
$arrays = array (

       // empty array
/*1*/  array(),

       // arrays with integer keys
       array(0 => "0"),
       array(1 => "1"),
       array(1 => "1", 2 => "2", 3 => "3", 4 => "4"),

       // arrays with float keys
/*5*/  array(2.3333 => "float"),
       array(1.2 => "f1", 3.33 => "f2",
             4.89999922839999 => "f3",
             33333333.333333 => "f4"),

       // arrays with string keys
/*7*/  array('\tHello' => 111, 're\td' => "color",
             '\v\fworld' => 2.2, 'pen\n' => 33),
       array("\tHello" => 111, "re\td" => "color",
             "\v\fworld" => 2.2, "pen\n" => 33),
       array("hello", $heredoc => "string"), // heredoc

       // array with object, unset variable and resource variable
/*10*/ array(new classA() => 11, @$unset_var => "hello", $fp => 'resource'),

       // array with mixed keys
/*11*/ array('hello' => 1, new classA() => 2, "fruit" => 2.2,
             $fp => 'resource', 133 => "int", 444.432 => "float",
             @$unset_var => "unset", $heredoc => "heredoc")
);

// array to be passsed to $arr2 argument
$arr2 = array(0 => 0, 2 => "float", 4 => "f3", 33333333 => "f4",
              "\tHello" => 111, 2.2, 'color', "Hello world" => "string", 
              "pen\n" => 33, new classA() => 11, 133 => "int");

// loop through each sub-array within $arrays to check the behavior of array_combine()
// same arrays are passed to both $keys and $values
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