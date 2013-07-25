<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

/*
 * Parse a load of URLs without specifying the component
 */
include_once(dirname(__FILE__) . '/urls.inc');

foreach ($urls as $url) {
	echo "\n--> $url: ";
	var_dump(parse_url($url));
}

echo "Done";
?>