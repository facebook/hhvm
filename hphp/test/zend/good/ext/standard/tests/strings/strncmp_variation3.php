<?hh
/* Prototype  : int strncmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-sensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncmp() with various lengths */
<<__EntryPoint>> function main(): void {
echo "*** Test strncmp() function: with different lengths ***\n";
/* definitions of required variables */
$str1 = "Hello, World\n";
$str2 = "Hello, world\n";

/* loop through to compare the strings, for various length values */
for($len = strlen($str1); $len >= 0; $len--) {
  var_dump( strncmp($str1, $str2, $len) );
}
echo "*** Done ***\n";
}
