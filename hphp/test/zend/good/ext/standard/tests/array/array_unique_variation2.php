<?hh
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array
 * Source code: ext/standard/array.c
*/

/*
* Passing different arrays to $input argument and testing whether
* array_unique() behaves in an expected way.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unique() : Passing different arrays to \$input argument ***\n";

/* Different heredoc strings passed as argument to arrays */
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

// arrays passed to $input argument
$inputs = vec[
/*1*/  vec[1, 2, 2, 1], // with default keys and numeric values
       vec[1.1, 2.2, 1.1], // with default keys & float values
       vec[false, true, false], // with default keys and boolean values
       vec[], // empty array
/*5*/  vec[NULL, null], // with NULL
       vec["a\v\f", "aaaa\r", "b", "aaaa\r", "\[\]\!\@\#\$\%\^\&\*\(\)\{\}"],  // with double quoted strings
       vec['a\v\f', 'aaaa\r', 'b', 'aaaa\r', '\[\]\!\@\#\$\%\^\&\*\(\)\{\}'],  // with single quoted strings
       dict["h1" => $blank_line, "h2" => $multiline_string, "h3" => $diff_whitespaces, 0 => $blank_line],  // with heredocs

       // associative arrays
/*9*/  dict[1 => "one", 2 => "two", 2 => "two"],  // explicit numeric keys, string values
       dict["one" => 1, "two" => 2, "1" => 1 ],  // string keys & numeric values
       dict[ 1 => 10, 2 => 20, 4 => 40, 5 => 10],  // explicit numeric keys and numeric values
       dict[ "one" => "ten", "two" => "twenty", "10" => "ten"],  // string key/value
       dict["one" => 1, 2 => "two", 4 => "four"],  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*14*/ dict['' => "NULL", '' => "null", "NULL" => NULL, "null" => null],
       dict[1 => "true", 0 => "false", "false" => false, "true" => true],
       dict["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       dict[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
/*18*/ dict['' => 1, "" => 2, '' => 3, '' => 4, 0 => 5, 1 => 6],
];

// loop through each sub-array of $inputs to check the behavior of array_unique()
$iterator = 1;
foreach($inputs as $input) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_unique($input, SORT_STRING) );
  $iterator++;
}

echo "Done";
}
