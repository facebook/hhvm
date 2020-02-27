<?hh
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys
 *              and the elements of the second as the corresponding values
 * Source code: ext/standard/array.c
*/

/*
* Passing different types of arrays to both $keys and $values arguments and testing whether
* array_combine() behaves in an expected way with the arguments passed to the function
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_combine() : Passing different types of arrays to both \$keys and \$values argument ***\n";
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

// arrays passed to $keys argument
$arrays = varray [
/*1*/  varray[1, 2], // with default keys and numeric values
       varray[1.1, 2.2], // with default keys & float values
       varray[false,true], // with default keys and boolean values
       varray[], // empty array
/*5*/  varray[NULL], // with NULL
       varray["a\v\f","aaaa\r","b","b\tbbb","c","\[\]\!\@\#\$\%\^\&\*\(\)\{\}"],  // with double quoted strings
       varray['a\v\f','aaaa\r','b','b\tbbb','c','\[\]\!\@\#\$\%\^\&\*\(\)\{\}'],  // with single quoted strings
       darray["h1" => $blank_line, "h2" => $multiline_string, "h3" => $diff_whitespaces, 0 => $numeric_string],  // with heredocs

       // associative arrays
/*9*/  darray[1 => "one", 2 => "two", 3 => "three"],  // explicit numeric keys, string values
       darray["one" => 1, "two" => 2, "three" => 3 ],  // string keys & numeric values
       darray[ 1 => 10, 2 => 20, 4 => 40, 3 => 30],  // explicit numeric keys and numeric values
       darray[ "one" => "ten", "two" => "twenty", "three" => "thirty"],  // string key/value
       darray["one" => 1, 2 => "two", 4 => "four"],  //mixed

       // associative array, containing null/empty/boolean values as key/value
/*14*/ darray[NULL => "NULL", null => "null", "NULL" => NULL, "null" => null],
       darray[true => "true", false => "false", "false" => false, "true" => true],
       darray["" => "emptyd", '' => 'emptys', "emptyd" => "", 'emptys' => ''],
       darray[1 => '', 2 => "", 3 => NULL, 4 => null, 5 => false, 6 => true],
       darray['' => 1, "" => 2, NULL => 3, null => 4, false => 5, true => 6],

       // array with repetative keys
/*19*/ darray["One" => 1, "two" => 2, "One" => 10, "two" => 20, "three" => 3]
];

// loop through each sub-array within $arrays to check the behavior of array_combine()
// same arrays are passed to both $keys and $values
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_combine($array, $array) );
  $iterator++;
}

echo "Done";
}
