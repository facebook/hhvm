<?hh
/* we get "Catchable fatal error: saying Object of class needle could not be
converted to string" by default when an object is passed instead of string:
The error can be  avoided by choosing the __toString magix method as follows: */

class mystring
{
  function __toString() :mixed{
    return "Hello, world";
  }
}

class needle
{
  function __toString() :mixed{
    return "world";
  }
}
<<__EntryPoint>>
function entrypoint_strstr(): void {
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
  $needles = vec[
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
  ];

  /* loop through to get the string starts with "needle" in $string */
  for( $i = 0; $i < count($needles); $i++ ) {
    echo "\n-- Iteration $i --\n";
    var_dump( strstr($string, $needles[$i]) );
  }


  echo "\n*** Testing Miscelleneous input data ***\n";

  echo "-- Passing objects as string and needle --\n";
  $obj_string = new mystring;
  $obj_needle = new needle;

  var_dump(strstr("$obj_string", "$obj_needle"));


  echo "\n-- passing an array as string and needle --\n";
  $needles = vec["hello", "?world", "!$%**()%**[][[[&@#~!"];
  try { var_dump( strstr($needles, $needles) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // won't work
  var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", (string)$needles[1]) );  // works
  var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", (string)$needles[2]) );  // works


  echo "\n-- passing Resources as string and needle --\n";
  $resource1 = fopen(__FILE__, "r");
  $resource2 = opendir(".");
  try { var_dump( strstr($resource1, $resource1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( strstr($resource1, $resource2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


  echo "\n-- Posiibilities with null --\n";
  var_dump( strstr("", NULL) );
  try { var_dump( strstr(NULL, NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
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
  try {
    var_dump( strstr("Hello, worldS", "$needleS") );  // won't work
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  /* String with curly braces, complex syntax */
  var_dump( strstr("Hello, worldS", "{$needle}S") );  // works
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
  try { var_dump( strstr() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }  // zero argument
  try { var_dump( strstr("") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }  // null argument
  try { var_dump( strstr($string) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }  // without "needle"
  try { var_dump( strstr("a", "b", "c") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected
  try { var_dump( strstr(NULL, "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  echo "\nDone";

  fclose($resource1);
  closedir($resource2);
}
