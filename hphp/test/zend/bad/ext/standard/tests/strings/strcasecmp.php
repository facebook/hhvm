<?php
/* Compares two strings in case-insensitive manner */

echo "#### Basic and Possible operations ####";
/* creating an array of strings to be compared */
$arrays = array(
           array("a", 'A', chr(128), chr(255), chr(256)),
           array("acc", "Acc", 'aC', "acCc", 'acd', "?acc", 'Acc!', "$!acc", ";acc"),
           array("1", "0", 0, "-1", -1, NULL, null, "", TRUE, true, FALSE, "string"),
           array(10.5, 1.5, 9.5, 11.5, 100.5, 10.5E1, -10.5, 10, 0.5)
          );

/* loop through to go each and every element in an array 
	and comparing the elements with one and other */
foreach($arrays as $str1_arr){
  echo "\n*** comparing the strings in an \n";
  print_r($str1_arr);
  for ($i=0; $i<count($str1_arr); $i++){
    echo "\nIteration $i\n";
    for($j=0; $j<count($str1_arr); $j++){
      echo "- strcasecmp of '$str1_arr[$i]' and '$str1_arr[$j]' is => ";
      var_dump(strcasecmp($str1_arr[$i], $str1_arr[$j]));
    }
  }
}



echo "\n#### Testing Miscelleneous inputs ####\n";

echo "--- Testing objects ---\n";
/* we get "Catchable fatal error: saying Object of class could not be converted
   to string" by default when an object is passed instead of string.
The error can be  avoided by choosing the __toString magix method as follows: */

class string1 {
  function __toString() {
    return "Hello, world";
  }
}
$obj_string1 = new string1;

class string2 {
  function __toString() {
    return "hello, world\0";
  }
}
$obj_string2 = new string2;

var_dump(strcasecmp("$obj_string1", "$obj_string2"));


echo "\n--- Testing arrays ---\n";
$str_arr = array("hello", "?world", "!$%**()%**[][[[&@#~!");
var_dump(strcasecmp("hello?world,!$%**()%**[][[[&@#~!",  $str_arr));
var_dump(strcasecmp("hello?world,!$%**()%**[][[[&@#~!", "$str_arr[1]"));
var_dump(strcasecmp("hello?world,!$%**()%**[][[[&@#~!", "$str_arr[2]"));

echo "\n--- Testing Resources ---\n";
$filename1 = "dummy.txt";
$filename2 = "dummy1.txt";

$file1 = fopen($filename1, "w");                // creating new file
$file2 = fopen($filename2, "w");                // creating new file

/* getting resource type for file handle */
$string1 = get_resource_type($file1);
$string2 = get_resource_type($file2);
$string3 = (int)get_resource_type($file2);

/* string1 and string2 of same "stream" type */
var_dump(strcasecmp($string1, $string2));            // int(0) 

/* string1 is of "stream" type & string3 is of "int" type */
var_dump(strcasecmp($string1, $string3));            // int(1) 

fclose($file1);                                 // closing the file "dummy.txt"
fclose($file2);                                 // closing the file "dummy1.txt"

unlink("$filename1");                           // deletes "dummy.txt"
unlink("$filename2");                           // deletes "dummy1.txt"


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
var_dump(strcasecmp($string, $string));
var_dump(strcasecmp($string, "xyz0123456789")); 
var_dump(strcasecmp($string, "&&&"));

echo "\n--- Testing a heredoc null string ---\n";
$str = <<<EOD
EOD;
var_dump(strcasecmp($str, "\0"));
var_dump(strcasecmp($str, NULL));
var_dump(strcasecmp($str, "0"));


echo "\n--- Testing simple and complex syntax strings ---\n";
$str = 'world';

/* Simple syntax */
var_dump(strcasecmp("Hello, world", "$str"));
var_dump(strcasecmp("Hello, world'S", "$str'S"));
var_dump(strcasecmp("Hello, worldS", "$strS"));

/* String with curly braces, complex syntax */
var_dump(strcasecmp("Hello, worldS", "${str}S"));
var_dump(strcasecmp("Hello, worldS", "{$str}S"));

echo "\n--- Testing binary safe and binary chars ---\n";
var_dump(strcasecmp("Hello\0world", "Hello"));
var_dump(strcasecmp("Hello\0world", "Helloworld"));
var_dump(strcasecmp("\x0", "\0"));
var_dump(strcasecmp("\000", "\0"));
var_dump(strcasecmp("\x00", ""));
var_dump(strcasecmp("\x00", NULL));
var_dump(strcasecmp("\000", NULL));

echo "\n--- Comparing long float values ---\n";
/* Here two different outputs, which depends on the rounding value 
   before converting to string. Here Precision = 12  */
var_dump(strcasecmp(10.55555555555555555555555555, 10.5555555556));   // int(0)
var_dump(strcasecmp(10.55555555555555555555555555, 10.555555556));    // int(-1)
var_dump(strcasecmp(10.55555555595555555555555555, 10.555555556));    // int(0)

echo "\n#### checking error conditions ####";
strcasecmp();
strcasecmp("");
strcasecmp("HI");
strcasecmp("Hi", "Hello", "World");

echo "Done\n";
?>