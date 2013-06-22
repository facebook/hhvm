<?php
/* Prototype  : array array_pad(array $input, int $pad_size, mixed $pad_value)
 * Description: Returns a copy of input array padded with pad_value to size pad_size
 * Source code: ext/standard/array.c
*/

/*
* Passing different arrays to $input argument and testing whether
* array_pad() behaves in an expected way with the other arguments passed to the function.
* The $pad_size and $pad_value arguments passed are fixed values.
*/

echo "*** Testing array_pad() : Passing different arrays to \$input argument ***\n";

/* Different heredoc strings */

// heredoc with blank line
$blank_line = <<<EOT


EOT;

// heredoc with multiline string
$multiline_string = <<<EOT
hello world
The big brown fox jumped over;
the lazy dog
This is a double quoted string
EOT;

// heredoc with different whitespaces
$diff_whitespaces = <<<EOT
hello\r world\t
1111\t\t != 2222\v\v
heredoc\ndouble quoted string. with\vdifferent\fwhite\vspaces
EOT;

// heredoc with quoted strings and numeric values
$numeric_string = <<<EOT
11 < 12. 123 >22
'single quoted string'
"double quoted string"
2222 != 1111.\t 0000 = 0000\n
EOT;

// different arrays to be passed to $input argument
$inputs = array (
/*1*/  array(1, 2), // with default keys and numeric values
       array(1.1, 2.2), // with default keys & float values
       array(false,true), // with default keys and boolean values
       array(), // empty array
/*5*/  array(NULL), // with NULL
       array("a\v\f", "aaaa\r", "b\tbbb", "\[\]\!\@\#\$\%\^\&\*\(\)\{\}"),  // with double quoted strings
       array('a\v\f', 'aaaa\r', 'b\tbbb', '\[\]\!\@\#\$\%\^\&\*\(\)\{\}'),  // with single quoted strings
       array("h1" => $blank_line, "h2" => $multiline_string, "h3" => $diff_whitespaces, $numeric_string),  // with heredocs

       // associative arrays
/*9*/  array(1 => "one", 2 => "two", 3 => "three"),  // explicit numeric keys, string values
       array("one" => 1, "two" => 2, "three" => 3 ),  // string keys & numeric values
       array( 1 => 10, 2 => 20, 4 => 40, 3 => 30),  // explicit numeric keys and numeric values
       array( "one" => "ten", "two" => "twenty", "three" => "thirty"),  // string key/value
       array("one" => 1, 2 => "two", 4 => "four"),  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*14*/ array(NULL => "NULL", null => "null", "NULL" => NULL, "null" => null),
       array(true => "true", false => "false", "false" => false, "true" => true),
       array("" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''),
       array(1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true),
       array('' => 1, "" => 2, NULL => 3, null => 4, false => 5, true => 6),

       // array with repetative keys
/*19*/ array("One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3)
);

// initialize the $pad_size and $pad_value arguments
$pad_size = 6;
$pad_value = "HELLO";

// loop through each sub-array within $inputs to check the behavior of array_pad()
$iterator = 1;
foreach($inputs as $input) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_pad($input, $pad_size, $pad_value) );  // positive 'pad_size'
  var_dump( array_pad($input, -$pad_size, $pad_value) );  // negative 'pad_size'
  $iterator++;
};

echo "Done";
?>