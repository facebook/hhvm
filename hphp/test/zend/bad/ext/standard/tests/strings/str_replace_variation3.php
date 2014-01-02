<?php
/*
  Prototype: mixed str_replace(mixed $search, mixed $replace,
                               mixed $subject [, int &$count]);
  Description: Replace all occurrences of the search string with
               the replacement string
*/


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

var_dump(str_replace("a", array("q", "q", "c"), array("aaa"), $count));
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


fclose($resource1);
closedir($resource2);

?>
===DONE===