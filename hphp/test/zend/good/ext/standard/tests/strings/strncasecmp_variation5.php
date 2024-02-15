<?hh
/* Prototype  : int strncasecmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-insensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncasecmp() function with the unexpected values, and giving the same strings for 'str1' and 'str2' */

/* declaring a class */
class sample  {
  public function __toString() :mixed{
  return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Test strncasecmp() function: unexpected values for 'len' ***\n";

/* definition of required variables */
$str1 = "Hello, World\n";
$str2 = "Hello, World\n";


/* get resource handle */
$file_handle = fopen(__FILE__, "r");


/* array with different values */
$lengths =  vec[
  /* hexadecimal values */
  0x12,

  /* octal values */
  012,
];

/* loop through each element of the array and check the working of strncasecmp() */
$counter = 1;
for($index = 0; $index < count($lengths); $index ++) {
  $len = $lengths[$index];
  echo "-- Iteration $counter --\n";
  try { var_dump( strncasecmp($str1, $str2, $len) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}
fclose($file_handle);

echo "*** Done ***\n";
}
