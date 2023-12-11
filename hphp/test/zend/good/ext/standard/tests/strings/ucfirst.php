<?hh
class mystring { function __toString() :mixed{ return "hello, world"; } }
<<__EntryPoint>> function main(): void {
/* Make a string's first character uppercase */
echo "#### Basic and Various operations ####\n";
$str_array = vec[
  "testing ucfirst.",
  "1.testing ucfirst",
  "hELLO wORLD",
  'hELLO wORLD',
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
foreach ($str_array as $string) {
  try { var_dump( ucfirst($string) ); } catch (Exception $e) { var_dump($e->getMessage()); }
}



echo "\n#### Testing Miscelleneous inputs ####\n";

echo "--- Testing arrays ---";
$str_arr = vec["hello", "?world", "!$%**()%**[][[[&@#~!", vec[]];
try { var_dump( ucfirst($str_arr) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n--- Testing objects ---\n";
// we get "Catchable fatal error: saying Object of class could not be converted
// to string" by default when an object is passed instead of string:
// The error can be  avoided by choosing the __toString magix method as follows:

$obj_string = new mystring;

var_dump(ucfirst("$obj_string"));


echo "\n--- Testing Resources ---\n";
$filename1 = sys_get_temp_dir().'/'."dummy-ucfirst.txt";
$file1 = fopen($filename1, "w");                // creating new file

/* getting resource type for file handle */
$string1 = get_resource_type($file1);
$string2 = (int)get_resource_type($file1);      // converting stream type to int

/* $string1 is of "stream" type */
var_dump(ucfirst($string1));

fclose($file1);                                 // closing the file "dummy-ucfirst.txt"
unlink("$filename1");                           // deletes "dummy-ucfirst.txt"


echo "\n--- Testing a longer and heredoc string ---\n";
$string = <<<EOD
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
@#$%^&**&^%$#@!~:())))((((&&&**%$###@@@!!!~~~~@###$%^&*
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
EOD;
var_dump(ucfirst($string));

echo "\n--- Testing a heredoc null string ---\n";
$str = <<<EOD
EOD;
var_dump(ucfirst($str));







echo "\n--- Testing simple and complex syntax strings ---\n";
$str = 'world';

/* Simple syntax */
var_dump(ucfirst("$str"));
var_dump(ucfirst("$str'S"));
try {
    var_dump(ucfirst("$strS"));
} catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
}


/* String with curly braces, complex syntax */
var_dump(ucfirst("{$str}S"));

echo "\n--- Nested ucfirst() ---\n";
var_dump(ucfirst(ucfirst("hello")));


echo "\n#### error conditions ####";
/* Zero arguments */
try { ucfirst(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
/* More than expected no. of args */
try { ucfirst($str_array[0], $str_array[1]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { ucfirst((int)10, (int)20); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
