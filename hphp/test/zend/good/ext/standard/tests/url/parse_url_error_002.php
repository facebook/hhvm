<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

echo "*** Testing parse_url() : error conditions: url component specifier out of range ***\n";
$url = 'http://secret:hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123';

echo "--> Below range:";
var_dump(parse_url($url, -1));

echo "\n\n--> Above range:";
var_dump(parse_url($url, 99));

echo "Done"
?>