<?hh
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is
                specified, break the string down into chunks each
                split_length characters long.
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different integer values for 'split_length' argument to str_split()
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : different intger values for 'split_length' ***\n";
//Initialise variables
$str = 'This is a string with 123 & escape char \t';

//different values for 'split_length'
$values = vec[
  0,
  1,
  -123,  //negative integer
  0234,  //octal number
  0x1A,  //hexadecimal number
  2147483647,  //max positive integer number
  2147483648,  //max positive integer+1
  -2147483648,  //min negative integer
];

//loop through each element of $values for 'split_length'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( str_split($str, $values[$count]) );
}
echo "Done";
}
