<?php
/* Prototype: string strstr ( string $haystack, string $needle );
   Description: Find first occurrence of a string 
   and reurns the rest of the string from that string 
*/

echo "*** Testing basic functionality of strstr() ***\n";
var_dump( strstr("test string", "test") );
var_dump( strstr("test string", "string") );
var_dump( strstr("test string", "strin") );
var_dump( strstr("test string", "t s") );
var_dump( strstr("test string", "g") );
var_dump( md5(strstr("te".chr(0)."st", chr(0))) );
var_dump( strstr("tEst", "test") );
var_dump( strstr("teSt", "test") );
var_dump( @strstr("", "") );
var_dump( @strstr("a", "") );
var_dump( @strstr("", "a") );


echo "\n*** Testing strstr() with various needles ***";
$string = 
"Hello world,012033 -3.3445     NULL TRUE FALSE\0 abcd\xxyz \x000 octal\n 
abcd$:Hello world";

/* needles in an array to get the string starts with needle, in $string */
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

/* loop through to get the string starts with "needle" in $string */
for( $i = 0; $i < count($needles); $i++ ) {
  echo "\n-- Iteration $i --\n";
  var_dump( strstr($string, $needles[$i]) );
}  

	
echo "\n*** Testing Miscelleneous input data ***\n";

echo "-- Passing objects as string and needle --\n";
/* we get "Catchable fatal error: saying Object of class needle could not be 
converted to string" by default when an object is passed instead of string:
The error can be  avoided by choosing the __toString magix method as follows: */

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

var_dump(strstr("$obj_string", "$obj_needle"));	


echo "\n-- passing an array as string and needle --\n";
$needles = array("hello", "?world", "!$%**()%**[][[[&@#~!");
var_dump( strstr($needles, $needles) );  // won't work
var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", "$needles[1]") );  // works
var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", "$needles[2]") );  // works


echo "\n-- passing Resources as string and needle --\n"; 
$resource1 = fopen(__FILE__, "r");
$resource2 = opendir(".");
var_dump( strstr($resource1, $resource1) );
var_dump( strstr($resource1, $resource2) );


echo "\n-- Posiibilities with null --\n";
var_dump( strstr("", NULL) );
var_dump( strstr(NULL, NULL) );
var_dump( strstr("a", NULL) );
var_dump( strstr("/x0", "0") );  // Hexadecimal NUL

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
var_dump( strstr($string, "abcd") );
var_dump( strstr($string, "1234") );		

echo "\n-- A heredoc null string --\n";
$str = <<<EOD
EOD;
var_dump( strstr($str, "\0") );
var_dump( strstr($str, NULL) );
var_dump( strstr($str, "0") );


echo "\n-- simple and complex syntax strings --\n";
$needle = 'world';

/* Simple syntax */
var_dump( strstr("Hello, world", "$needle") );  // works 
var_dump( strstr("Hello, world'S", "$needle'S") );  // works
var_dump( strstr("Hello, worldS", "$needleS") );  // won't work 

/* String with curly braces, complex syntax */
var_dump( strstr("Hello, worldS", "${needle}S") );  // works
var_dump( strstr("Hello, worldS", "{$needle}S") );  // works


echo "\n-- complex strings containing other than 7-bit chars --\n";
$str = chr(0).chr(128).chr(129).chr(234).chr(235).chr(254).chr(255);
echo "- Positions of some chars in the string '$str' are as follows -\n";
echo chr(128)." => "; 
var_dump( strstr($str, chr(128)) );		
echo chr(255)." => "; 
var_dump( strstr($str, chr(255)) );
echo chr(256)." => "; 
var_dump( strstr($str, chr(256)) ); 

echo "\n*** Testing error conditions ***";
var_dump( strstr($string, ""));
var_dump( strstr() );  // zero argument
var_dump( strstr("") );  // null argument 
var_dump( strstr($string) );  // without "needle"
var_dump( strstr("a", "b", "c") );  // args > expected
var_dump( strstr(NULL, "") );

echo "\nDone";

fclose($resource1);
closedir($resource2);
?>