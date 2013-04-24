<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

/*
 *  check values of URL related constants
 */
foreach(get_defined_constants() as $constantName => $constantValue) {
	if (strpos($constantName, 'PHP_URL')===0) {
		echo "$constantName: $constantValue \n";
	}
}

echo "Done";
?>