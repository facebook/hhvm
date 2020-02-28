<?hh
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments
 * Source code: ext/standard/array.c
*/

/*
* Passing different types of arrays to $arr2 argument and testing whether
* array_intersect() behaves in expected way with the other arguments passed to the function.
* The $arr1 argument is a fixed array.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect() : Passing different types of arrays to \$arr2 argument ***\n";

/* Different heredoc strings passed as argument to $arr2 */
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

// array to be passsed to $arr1 argument
$arr1 = darray[
  0 => 1, 1 => 1.1, 2 => "hello", 3 => "one", 4 => NULL, 5 => 2,
  6 => 'world', 7 => true, 8 => false, false => 5, 9 => 'aaaa\r', 10 => "aaaa\r",
  11 => $numeric_string, 12 => $diff_whitespaces,
  "one" => "ten", 4 => "four", "two" => 2, 2 => "two",
  13 => '', null => "null", '' => 'emptys'
];

// arrays to be passed to $arr2 argument
$arrays = varray [
/*1*/  varray[1, 2], // array with default keys and numeric values
       varray[1.1, 2.2], // array with default keys & float values
       varray[false,true], // array with default keys and boolean values
       varray[], // empty array
/*5*/  varray[NULL], // array with NULL
       varray["a\v\f","aaaa\r","b","b\tbbb","c","\[\]\!\@\#\$\%\^\&\*\(\)\{\}"],  // array with double quoted strings
       varray['a\v\f','aaaa\r','b','b\tbbb','c','\[\]\!\@\#\$\%\^\&\*\(\)\{\}'],  // array with single quoted strings
       varray[$blank_line, $multiline_string, $diff_whitespaces, $numeric_string],  // array with heredocs

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

// loop through each sub-array within $arrrays to check the behavior of array_intersect()
$iterator = 1;
foreach($arrays as $arr2) {
  echo "-- Iteration $iterator --\n";

  // Calling array_intersect() with default arguments
  var_dump( array_intersect($arr1, $arr2) );

  // Calling array_intersect() with more arguments
  // additional argument passed is the same as $arr1 argument
  var_dump( array_intersect($arr1, $arr2, $arr1) );
  $iterator++;
}

echo "Done";
}
