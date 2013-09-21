<?php

/* Prototype  : string convert_cyr_string  ( string $str  , string $from  , string $to  )
 * Description: Convert from one Cyrillic character set to another
 * Source code: ext/standard/string.c
*/

echo "*** Testing convert_cyr_string() : basic functionality ***\n";

$str = "Convert from one Cyrillic character set to another.";

echo "\n-- First try some simple English text --\n";
var_dump(bin2hex(convert_cyr_string($str, 'w', 'k')));
var_dump(bin2hex(convert_cyr_string($str, 'w', 'i')));


echo "\n-- Now try some of characters in 128-255 range --\n";

for ($i = 128; $i < 256; $i++) {
	$str = chr($i);
	echo "$i: " . bin2hex(convert_cyr_string($str, 'w', 'k')) . "\n";
}

?>
===DONE===