<?hh
/* Prototype  : int strncmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-sensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncmp() function with single quoted strings for 'str1', 'str2' */
<<__EntryPoint>> function main(): void {
echo "*** Test strncmp() function: with single quoted strings ***\n";
$strings = vec[
  'Hello, World',
  'hello, world',
  'HELLO, WORLD',
  'Hello, World\n'
];
/* loop through to compare each string with the other string */
$count = 1;
for($index1 = 0; $index1 < count($strings); $index1++) {
  echo "-- Iteration $count --\n";
  for($index2 = 0; $index2 < count($strings); $index2++) {
    var_dump( strncmp( $strings[$index1], $strings[$index2], (strlen($strings[$index1]) + 1) ) );
  }
  $count ++;
}
echo "*** Done ***\n";
}
