<?php
/* 
  Prototype: mixed str_replace(mixed $search, mixed $replace, 
                               mixed $subject [, int &$count]);
  Description: Replace all occurrences of the search string with 
               the replacement string
*/

echo "\n*** Testing str_replace() on basic operations ***\n";

var_dump( str_replace("", "", "") );

var_dump( str_replace("e", "b", "test") );

var_dump( str_replace("", "", "", $count) );
var_dump( $count );

var_dump( str_replace("q", "q", "q", $count) );
var_dump( $count );

var_dump( str_replace("long string here", "", "", $count) );
var_dump( $count );

$fp = fopen( __FILE__, "r" );
$fp_copy = $fp; 
var_dump( str_replace($fp_copy, $fp_copy, $fp_copy, $fp_copy) );
var_dump( $fp_copy );
fclose($fp);

echo "\n*** Testing str_replace() with various search values ***";
$search_arr = array( TRUE, FALSE, 1, 0, -1, "1", "0", "-1",  NULL, 
                     array(), "php", "");

$i = 0;
/* loop through to replace the matched elements in the array */
foreach( $search_arr as $value ) {
  echo "\n-- Iteration $i --\n";
  /* replace the string in array */
  var_dump( str_replace($value, "FOUND", $search_arr, $count) ); 
  var_dump( $count );
  $i++;
}

echo "\n*** Testing str_replace() with various subjects ***";
$subject = "Hello, world,0120333.3445-1.234567          NULL TRUE FALSE\000
 	    \x000\xABCD\0abcd \xXYZ\tabcd $$@#%^&*!~,.:;?: !!Hello, World 
	    ?Hello, World chr(0).chr(128).chr(234).chr(65).chr(255).chr(256)";

/* needles in an array to be compared in the string $string */
$search_str = array ( 
  "Hello, World",
  'Hello, World',
  '!!Hello, World',
  "??Hello, World",
  "$@#%^&*!~,.:;?",
  "123",
  123,
  "-1.2345",
  -1.2344,
  "abcd",
  'XYZ',
  NULL,
  "NULL",
  "0",
  0,
  "",
  " ",
  "\0",
  "\x000",
  "\xABC",
  "\0000",
  ".3",
  TRUE,
  "TRUE",
  "1",
  1,
  FALSE,
  "FALSE",
  " ",
  "          ",
  'b',
  '\t',
  "\t",
  chr(128).chr(234).chr(65).chr(255).chr(256),
  $subject
);

/* loop through to get the  $string */
for( $i = 0; $i < count($search_str); $i++ ) {
  echo "\n--- Iteration $i ---";
  echo "\n-- String after replacing the search value is => --\n";
  var_dump( str_replace($search_str[$i], "FOUND", $subject, $count) );
  echo "-- search string has found '$count' times\n";
}
  

echo "\n*** Testing Miscelleneous input data ***\n";
/*  If replace has fewer values than search, then an empty 
    string is used for the rest of replacement values */
var_dump( str_replace(array("a", "a", "b"), 
		      array("q", "q"), 
		      "aaabb", $count
		     )
	);
var_dump($count);
var_dump( str_replace(array("a", "a", "b"), 
                      array("q", "q"), 
                      array("aaa", "bbb", "ccc"), 
                      $count
                     )
        );
var_dump($count);


echo "\n-- Testing objects --\n";
/* we get "Catchable fatal error: saying Object of class could not be converted
        to string" by default, when an object is passed instead of string:
The error can be  avoided by choosing the __toString magix method as follows: */

class subject 
{
  function __toString() {
    return "Hello, world";
  }
}
$obj_subject = new subject;

class search 
{
  function __toString() {
    return "Hello, world";
  }
}
$obj_search = new search;

class replace 
{
  function __toString() {
    return "Hello, world";
  }
}
$obj_replace = new replace;

var_dump(str_replace("$obj_search", "$obj_replace", "$obj_subject", $count));
var_dump($count);


echo "\n-- Testing arrays --\n";
var_dump(str_replace(array("a", "a", "b"), "multi", "aaa", $count));
var_dump($count);

var_dump(str_replace( array("a", "a", "b"),
                      array("q", "q", "c"), 
                      "aaa", $count
                    )
);
var_dump($count);

var_dump(str_replace( array("a", "a", "b"),
                      array("q", "q", "c"), 
                      array("aaa", "bbb"), 
                      $count
                    )
);
var_dump($count);

var_dump(str_replace("a", array("q", "q", "c"), array("aaa", "bbb"), $count));
var_dump($count);

var_dump(str_replace("a", 1, array("aaa", "bbb"), $count));
var_dump($count);

var_dump(str_replace(1, 3, array("aaa1", "2bbb"), $count));
var_dump($count);


echo "\n-- Testing Resources --\n";
$resource1 = fopen( __FILE__, "r" );
$resource2 = opendir( "." );
var_dump(str_replace("stream", "FOUND", $resource1, $count)); 
var_dump($count);
var_dump(str_replace("stream", "FOUND", $resource2, $count));
var_dump($count);


echo "\n-- Testing a longer and heredoc string --\n";
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

var_dump( str_replace("abcdef", "FOUND", $string, $count) );
var_dump( $count );

echo "\n-- Testing a heredoc null string --\n";
$str = <<<EOD
EOD;
var_dump( str_replace("", "FOUND", $str, $count) );
var_dump( $count );


echo "\n-- Testing simple and complex syntax strings --\n";
$str = 'world';

/* Simple syntax */
var_dump( str_replace("world", "FOUND", "$str") );
var_dump( str_replace("world'S", "FOUND", "$str'S") );
var_dump( str_replace("worldS", "FOUND", "$strS") );

/* String with curly braces, complex syntax */
var_dump( str_replace("worldS", "FOUND", "${str}S") );
var_dump( str_replace("worldS", "FOUND", "{$str}S") );


echo "\n*** Testing error conditions ***";
/* Invalid arguments */
var_dump( str_replace() );
var_dump( str_replace("") );
var_dump( str_replace(NULL) );
var_dump( str_replace(1, 2) );
var_dump( str_replace(1,2,3,$var,5) );

fclose($resource1);
closedir($resource2);
echo "Done\n";

?>