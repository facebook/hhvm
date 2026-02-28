<?hh
/* Prototype  : int strncasecmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-insensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncasecmp() function with the unexpected inputs for 'str1' */

/* declaring a class */
class sample  {
  public function __toString() :mixed{
  return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strncasecmp() function: with unexpected values for 'str1' ***\n";

/* get resource handle */
$file_handle = fopen(__FILE__, "r");


/* array with different values */
$values =  vec[
  /* empty string */
  "",
  '',
];

/* loop through each element of the array and check the working of strncasecmp() */
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str1 = $values[$index];
  try { $len = strlen($values[$index]) + 1; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( strncasecmp($str1, "string", $len) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***\n";
}
