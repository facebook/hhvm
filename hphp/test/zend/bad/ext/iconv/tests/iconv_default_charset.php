<?php
echo "*** Testing default_charset handling ***\n";

echo "--- Get php.ini values ---\n";
var_dump(ini_get('default_charset'),
		 ini_get('internal_encoding'),
		 ini_get('input_encoding'),
		 ini_get('output_encoding'),
		 ini_get('iconv.internal_encoding'),
		 ini_get('iconv.input_encoding'),
		 ini_get('iconv.output_encoding'));

echo "\n--- Altering encodings ---\n";
var_dump(ini_set('default_charset', 'ISO-8859-1'));

echo "\n--- results of alterations ---\n";
var_dump(ini_get('default_charset'),
		 ini_get('internal_encoding'),
		 ini_get('input_encoding'),
		 ini_get('output_encoding'),
		 ini_get('iconv.internal_encoding'),
		 ini_get('iconv.input_encoding'),
		 ini_get('iconv.output_encoding'));

/*
echo "\n--- Altering encodings ---\n";
var_dump(ini_set('default_charset', 'ISO-8859-1'),
		 ini_set('internal_encoding'),
		 ini_set('input_encoding'),
		 ini_set('output_encoding'),
		 ini_set('iconv.internal_encoding'),
		 ini_set('iconv.input_encoding'),
		 ini_set('iconv.output_encoding'));
*/

echo "Done";
?>
