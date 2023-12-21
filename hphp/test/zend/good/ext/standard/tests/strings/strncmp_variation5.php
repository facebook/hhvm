<?hh
/* Prototype  : int strncmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-sensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncmp() function with the length as all types, and giving the same strings for 'str1' and 'str2' */

/* declaring a class */
class sample  {
  public function __toString() :mixed{
  return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Test strncmp() function: by supplying all types for 'len' ***\n";

/* definition of required variables */
$str1 = "Hello, World\n";
$str2 = "Hello, World\n";


/* get resource handle */
$file_handle = fopen(__FILE__, "r");


/* array with different values */
$lengths =  vec[
  /* integer values */
  0,
  1,
  12345,

  /* float values */
  10.5,
  10.5e10,
  10.6E-10,
  .5,

  /* hexadecimal values */
  0x12,

  /* octal values */
  012,
  01.2,

  /* array values */
  vec[],
  vec[0],
  vec[1],
  vec[1, 2],
  dict['color' => 'red', 'item' => 'pen'],

  /* boolean values */
  true,
  false,
  TRUE,
  FALSE,

  /* nulls */
  NULL,
  null,

  /* empty string */
  "",
  '',



  /* resource */
  $file_handle,

  /* object */
  new sample()
];

/* loop through each element of the array and check the working of strncmp() */
$counter = 1;
for($index = 0; $index < count($lengths); $index ++) {
  $len = $lengths[$index];
  echo "-- Iteration $counter --\n";
  try { var_dump( strncmp($str1, $str2, $len) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}
fclose($file_handle);

echo "*** Done ***\n";
}
