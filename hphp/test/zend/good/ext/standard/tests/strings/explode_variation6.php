<?php

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing explode() function: misc tests ***\n";

$str = "one\x00two\x00three\x00four";

echo "\n-- positive limit with null separator --\n";
$e = test_explode("\x00", $str, 2);

echo "\n-- negative limit (since PHP 5.1) with null separator --\n";
$e = test_explode("\x00", $str, -2);

echo "\n-- unknown string --\n";
$e = test_explode("fred", $str,1);

echo "\n-- limit = 0 --\n";
$e = test_explode("\x00", $str, 0);

echo "\n-- limit = -1 --\n";
$e = test_explode("\x00", $str, -1);

echo "\n-- large limit = -100 --\n";
$e = test_explode("\x00", $str, 100);

function test_explode($delim, $string, $limit)
{
	$e = explode($delim, $string, $limit);
	foreach ( $e as $v) 
	{
		var_dump(bin2hex($v));
	}	
}
?>
===DONE===