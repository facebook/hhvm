<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
* Passing different arrays to $arr1 argument and testing whether
* array_merge_recursive() behaves in an expected way.
*/

echo "*** Testing array_merge_recursive() : Passing different arrays to \$arr1 argument ***\n";

/* Different heredoc strings */

// heredoc with blank line
$blank_line = <<<EOT


EOT;

// heredoc with multiline string
$multiline_string = <<<EOT
hello world
The quick brown fox jumped over;
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

// arrays passed to $arr1 argument
$arrays = array (
/*1*/  array(1, 2,), // with default keys and numeric values
       array(1.1, 2.2), // with default keys & float values
       array(false, true), // with default keys and boolean values
       array(), // empty array
/*5*/  array(NULL), // with NULL
       array("a\v\f", "aaaa\r", "b", "\[\]\!\@\#\$\%\^\&\*\(\)\{\}"),  // with double quoted strings
       array('a\v\f', 'aaaa\r', 'b', '\[\]\!\@\#\$\%\^\&\*\(\)\{\}'),  // with single quoted strings
       array("h1" => $blank_line, "h2" => $multiline_string, "h3" => $diff_whitespaces),  // with heredocs

       // associative arrays
/*9*/  array(1 => "one", 2 => "two"),  // explicit numeric keys, string values
       array("one" => 1, "two" => 2, "1" => 1 ),  // string keys & numeric values
       array( 1 => 10, 2 => 20, 4 => 40),  // explicit numeric keys and numeric values
       array( "one" => "ten", "two" => "twenty"),  // string key/value
       array("one" => 1, 2 => "two", 4 => "four"),  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*14*/ array(NULL => "NULL", null => "null", "NULL" => NULL, "null" => null),
       array(true => "true", false => "false", "false" => false, "true" => true),
       array("" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''),
       array(1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true),
       array('' => 1, "" => 2, NULL => 3, null => 4, false => 5, true => 6),

       // array containing embedded arrays
/*15*/ array("str1", "array" => array("hello", 'world'), array(1, 2))
);

// initialise the second argument
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
  
echo "Done";
?>