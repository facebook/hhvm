<?php
/* Make a string's first character uppercase */

echo "#### Basic and Various operations ####\n";
$str_array = array(
		    "TesTing lcfirst.",
 		    "1.testing lcfirst",
		    "HELLO wORLD",
		    'HELLO wORLD',
                    "\0",		// Null 
                    "\x00",		// Hex Null
                    "\x000",
                    "abcd",		// double quoted string
                    'xyz',		// single quoted string
                    string,		// without quotes
                    "-3",
                    -3,
                    '-3.344',
                    -3.344,
                    NULL,
                    "NULL",
                    "0",
                    0,
                    TRUE,		// bool type
                    "TRUE",
                    "1",
                    1,
                    1.234444,
                    FALSE,
                    "FALSE",
                    " ",
                    "     ",
                    'b',		// single char
                    '\t',		// escape sequences
                    "\t",
                    "12",
                    "12twelve",		// int + string
	     	  );
/* loop to test working of lcfirst with different values */
foreach ($str_array as $string) {
  var_dump( lcfirst($string) );
}



echo "\n#### Testing Miscelleneous inputs ####\n";

echo "--- Testing arrays ---";
$str_arr = array("Hello", "?world", "!$%**()%**[][[[&@#~!", array());
var_dump( lcfirst($str_arr) );  

echo "\n--- Testing lowercamelcase action call example ---\n";
class Setter {
    
    protected $vars = array('partnerName' => false);
    
    public function __call($m, $v) {
        if (stristr($m, 'set')) {
            $action = lcfirst(substr($m, 3));
            $this->$action = $v[0];
        }
    }

    public function __set($key, $value) {
        if (array_key_exists($key, $this->vars)) {
            $this->vars[$key] = $value;
        }
    }

    public function __get($key) {
        if (array_key_exists($key, $this->vars)) {
            return $this->vars[$key];
        }
    }
}

$class = new Setter();
$class->setPartnerName('partnerName');
var_dump($class->partnerName);

echo "\n--- Testing objects ---\n";
/* we get "Catchable fatal error: saying Object of class could not be converted
        to string" by default when an object is passed instead of string:
The error can be  avoided by choosing the __toString magix method as follows: */

class string {
  function __toString() {
    return "Hello world";
  }
}
$obj_string = new string;

var_dump(lcfirst("$obj_string"));


echo "\n--- Testing Resources ---\n";
$filename1 = "dummy.txt";
$file1 = fopen($filename1, "w");                // creating new file

/* getting resource type for file handle */
$string1 = get_resource_type($file1);
$string2 = (int)get_resource_type($file1);      // converting stream type to int

/* $string1 is of "stream" type */
var_dump(lcfirst($string1)); 

/* $string2 holds a value of "int(0)" */
var_dump(lcfirst($string2));

fclose($file1);                                 // closing the file "dummy.txt"
unlink("$filename1");                           // deletes "dummy.txt"


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
var_dump(lcfirst("$strS"));

/* String with curly braces, complex syntax */
var_dump(lcfirst("${str}S"));
var_dump(lcfirst("{$str}S"));

echo "\n--- Nested lcfirst() ---\n";
var_dump(lcfirst(lcfirst("hello")));


echo "\n#### error conditions ####";
/* Zero arguments */
lcfirst();
/* More than expected no. of args */
lcfirst($str_array[0], $str_array[1]);
lcfirst((int)10, (int)20);

echo "Done\n";
?>