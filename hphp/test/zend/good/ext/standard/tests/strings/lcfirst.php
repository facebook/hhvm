<?hh
/* Make a string's first character uppercase */

class mystring {
  function __toString() :mixed{
    return "Hello world";
  }
}

<<__EntryPoint>> function main(): void {
echo "#### Basic and Various operations ####\n";
$str_array = vec[
            "TesTing lcfirst.",
            "1.testing lcfirst",
            "HELLO wORLD",
            'HELLO wORLD',
                    "\0",       // Null
                    "\x00",     // Hex Null
                    "\x000",
                    "abcd",     // double quoted string
                    'xyz',      // single quoted string
                    "-3",
                    -3,
                    '-3.344',
                    -3.344,
                    NULL,
                    "NULL",
                    "0",
                    0,
                    TRUE,       // bool type
                    "TRUE",
                    "1",
                    1,
                    1.234444,
                    FALSE,
                    "FALSE",
                    " ",
                    "     ",
                    'b',        // single char
                    '\t',       // escape sequences
                    "\t",
                    "12",
                    "12twelve",     // int + string
              ];
/* loop to test working of lcfirst with different values */
foreach ($str_array as $string) {
  try { var_dump( lcfirst($string) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}



echo "\n#### Testing Miscelleneous inputs ####\n";

echo "--- Testing arrays ---";
$str_arr = vec["Hello", "?world", "!$%**()%**[][[[&@#~!", vec[]];
try { var_dump( lcfirst($str_arr) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n--- Testing objects ---\n";
/* we get "Catchable fatal error: saying Object of class could not be converted
 * to string" by default when an object is passed instead of string, but that
 * error can be avoided by defining a __toString magix method (see "mystring").
 */
$obj_string = new mystring;
var_dump(lcfirst("$obj_string"));

echo "\n--- Testing Resources ---\n";
$filename1 = sys_get_temp_dir().'/'."dummy-lcfirst.txt";
$file1 = fopen($filename1, "w");                // creating new file
/* getting resource type for file handle */
$string1 = get_resource_type($file1);
$string2 = (int)get_resource_type($file1);      // converting stream type to int

/* $string1 is of "stream" type */
var_dump(lcfirst($string1));

/* $string2 holds a value of "int(0)" */
try { var_dump(lcfirst($string2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($file1);                                 // closing the file "dummy-lcfirst.txt"
unlink("$filename1");                           // deletes "dummy-lcfirst.txt"


echo "\n--- Testing a longer and heredoc string ---\n";
$string = <<<EOD
Abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
@#$%^&**&^%$#@!~:())))((((&&&**%$###@@@!!!~~~~@###$%^&*
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
EOD;
var_dump(lcfirst($string));

echo "\n--- Testing a heredoc null string ---\n";
$str = <<<EOD
EOD;
var_dump(lcfirst($str));


echo "\n--- Testing simple and complex syntax strings ---\n";
$str = 'world';

/* Simple syntax */
var_dump(lcfirst("$str"));
var_dump(lcfirst("$str'S"));
try {
  var_dump(lcfirst("$strS"));
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}


/* String with curly braces, complex syntax */
var_dump(lcfirst("{$str}S"));

echo "\n--- Nested lcfirst() ---\n";
var_dump(lcfirst(lcfirst("hello")));


echo "\n#### error conditions ####";
/* Zero arguments */
try { lcfirst(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
/* More than expected no. of args */
try { lcfirst($str_array[0], $str_array[1]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { lcfirst((int)10, (int)20); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
