<?php
/* Prototype  : proto array get_headers(string url[, int format])
 * Description: Fetches all the headers sent by the server in response to a HTTP request
 * Source code: ext/standard/url.c
 * Alias to functions:
 */

echo "*** Testing get_headers() : error conditions ***\n";
$url = 'http://php.net';

// Format argument as type String
echo "\n-- Testing get_headers() function with format argument as type string --\n";
var_dump( get_headers($url, "#PHPTestFest2009 Norway") );

// Format argument as type Array
echo "\n-- Testing get_headers() function with format argument as type array --\n";
var_dump( get_headers($url, array()) );

// Format argument as type Object
class testObject
{
}

$object = new testObject();
echo "\n-- Testing get_headers() function with format argument as type object --\n";
var_dump( get_headers($url, $object) );


echo "Done"
?>