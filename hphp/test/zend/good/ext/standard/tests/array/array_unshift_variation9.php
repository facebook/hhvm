<?php
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_unshift() by passing different
 * heredoc strings for $var argument that is prepended to the array 
 * passed through $array argument
*/

echo "*** Testing array_unshift() : heredoc strings for \$var argument ***\n";

// heredoc with empty value
$empty_string = <<<EOT
EOT;
  
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

// heredoc with numeric values
$numeric_string = <<<EOT
11 < 12. 123 >22
2222 != 1111.\t 0000 = 0000\n
EOT;

// heredoc with quote chars & slash
$quote_char_string = <<<EOT
This's a string with quotes:
"strings in double quote";
'strings in single quote';
this\line is single quoted /with\slashes
EOT;

// array to be passed to $array argument
$array = array('f' => "first", "s" => 'second', 1, 2.222);

// different heredoc strings to be passed to $var argument
$vars = array(
  $empty_string,
  $blank_line,
  $multiline_string,
  $diff_whitespaces,
  $numeric_string,
  $quote_char_string
);

// loop through the various elements of $arrays to test array_unshift()
$iterator = 1;
foreach($vars as $var) {
  echo "-- Iteration $iterator --\n";
  $temp_array = $array;  // assign $array to another temporary $temp_array

  /* with default argument */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  var_dump( array_unshift($temp_array, $var) );
  
  // dump the resulting array
  var_dump($temp_array);

  /* with all possible arguments */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift($temp_array, $var, "hello", 'world') );
  
  // dump the resulting array
  var_dump($temp_array);
  $iterator++;
}

echo "Done";
?>