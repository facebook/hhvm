<?php
/* Prototype: int strpos ( string $haystack, mixed $needle [, int $offset] );
   Description: Find position of first occurrence of a string
*/

echo "*** Testing basic functionality of strpos() ***\n";
var_dump( strpos("test string", "test") );
var_dump( strpos("test string", "string") );
var_dump( strpos("test string", "strin") );
var_dump( strpos("test string", "t s") );
var_dump( strpos("test string", "g") );
var_dump( strpos("te".chr(0)."st", chr(0)) );
var_dump( strpos("tEst", "test") );
var_dump( strpos("teSt", "test") );
var_dump( @strpos("", "") );
var_dump( @strpos("a", "") );
var_dump( @strpos("", "a") );
var_dump( @strpos("\\\\a", "\\a") );


echo "\n*** Testing strpos() to find various needles and a long string ***\n";
$string = 
"Hello world,012033 -3.3445     NULL TRUE FALSE\0 abcd\xxyz \x000 octal\n
abcd$:Hello world";

/* needles in an array to get the position of needle in $string */
$needles = array(
  "Hello world", 	
  "WORLD", 
  "\0", 
  "\x00", 
  "\x000", 
  "abcd", 
  "xyz", 
  "octal", 
  "-3", 
  -3, 
  "-3.344", 
  -3.344, 
  NULL, 
  "NULL",
  "0",
  0, 
  TRUE, 
  "TRUE",
  "1",
  1,
  FALSE,
  "FALSE",
  " ",
  "     ",
  'b',
  '\n',
  "\n",
  "12",
  "12twelve",
  $string
);

/* loop through to get the "needle" position in $string */
for( $i = 0; $i < count($needles); $i++ ) {
  echo "Position of '$needles[$i]' is => ";
  var_dump( strpos($string, $needles[$i]) );
}  


echo "\n*** Testing strpos() with possible variations in offset ***\n";
$offset_values = array (
  1,  // offset = 1
  "string",  // offset as string, converts to zero
  NULL,  // offset as string, converts to zero
  "",  // offset as string, converts to zero
  "12string",  // mixed string with int and chars
  "0",
  TRUE,
  NULL,
  FALSE,
  "string12",
  "12.3string",  // mixed string with float and chars
);

/* loop through to get the "needle" position in $string */
for( $i = 0; $i < count( $offset_values ); $i++ ) {
  echo "Position of 'Hello' with offset '$offset_values[$i]' is => ";
  var_dump( strpos($string, "Hello", $offset_values[$i]) );
}


echo "\n*** Testing Miscelleneous input data ***\n";

echo "-- Passing objects as string and needle --\n";
/* we get "Catchable fatal error: saying Object of class needle could not be
 converted to string" by default when an object is passed instead of string:
 The error can be avoided by choosing the __toString magix method as follows: */

class string 
{
  function __toString() {
    return "Hello, world";
  }
}
$obj_string = new string;

class needle 
{
  function __toString() {
    return "world";
  }
}
$obj_needle = new needle;

var_dump( strpos("$obj_string", "$obj_needle") );

echo "\n-- Passing an array as string and needle --\n";
$needles = array("hello", "?world", "!$%**()%**[][[[&@#~!");
var_dump( strpos($needles, $needles) );	 // won't work
var_dump( strpos("hello?world,!$%**()%**[][[[&@#~!", "$needles[1]") );	// works
var_dump( strpos("hello?world,!$%**()%**[][[[&@#~!", "$needles[2]") );	// works


echo "\n-- Passing Resources as string and needle --\n"; 
$resource1 = fopen(__FILE__, "r");
$resource2 = opendir(".");
var_dump( strpos($resource1, $resource1) );
var_dump( strpos($resource1, $resource2) );

echo "\n-- Posiibilities with null --\n";
var_dump( strpos("", NULL) );
var_dump( strpos(NULL, NULL) );
var_dump( strpos("a", NULL) );
var_dump( strpos("/x0", "0") );	 // Hexadecimal NUL

echo "\n-- A longer and heredoc string --\n";
$string = <<<EOD
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
EOD;
var_dump( strpos($string, "abcd") );
var_dump( strpos($string, "abcd", 72) );  // 72 -> "\n" in the first line
var_dump( strpos($string, "abcd", 73) );  // 73 -> "abcd" in the second line
var_dump( strpos($string, "9", (strlen($string)-1)) );

echo "\n-- A heredoc null string --\n";
$str = <<<EOD
EOD;
var_dump( strpos($str, "\0") );
var_dump( strpos($str, NULL) );
var_dump( strpos($str, "0") );


echo "\n-- simple and complex syntax strings --\n";
$needle = 'world';

/* Simple syntax */
var_dump( strpos("Hello, world", "$needle") );  // works 
var_dump( strpos("Hello, world'S", "$needle'S") );  // works
var_dump( strpos("Hello, worldS", "$needleS") );  // won't work 

/* String with curly braces, complex syntax */
var_dump( strpos("Hello, worldS", "${needle}S") );  // works
var_dump( strpos("Hello, worldS", "{$needle}S") );  // works


echo "\n-- complex strings containing other than 7-bit chars --\n";
$str = chr(0).chr(128).chr(129).chr(234).chr(235).chr(254).chr(255);
echo "-- Positions of some chars in the string '$str' are as follows --\n";
echo chr(128)." => "; 
var_dump( strpos($str, chr(128)) );		
echo chr(255)." => "; 
var_dump( strpos($str, chr(255), 3) );
echo chr(256)." => "; 
var_dump( strpos($str, chr(256)) );

echo "\n*** Testing error conditions ***";
var_dump( strpos($string, "") );
var_dump( strpos() );  // zero argument
var_dump( strpos("") );  // null argument 
var_dump( strpos($string) );  // without "needle"
var_dump( strpos("a", "b", "c", "d") );  // args > expected
var_dump( strpos($string, "test", strlen($string)+1) );  // offset > strlen()
var_dump( strpos($string, "test", -1) );  // offset < 0
var_dump( strpos(NULL, "") );

echo "\nDone";

fclose($resource1); 
closedir($resource2);
?>