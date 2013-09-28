<?php
/* Prototype  : string iconv(string in_charset, string out_charset, string str)
 * Description: Returns converted string in desired encoding 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test basic functionality of iconv()
 */

echo "*** Testing iconv() : basic functionality ***\n";

//All strings are the same when displayed in their respective encodings
$sjis_string = base64_decode(b'k/qWe4zqg2WDTINYg2eCxYK3gUIwMTIzNIJUglWCVoJXgliBQg==');
$euc_jp_string = base64_decode(b'xvzL3LjspcalraW5pcikx6S5oaMwMTIzNKO1o7ajt6O4o7mhow==');
$utf8_string = base64_decode(b'5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

echo "\n-- Convert to EUC-JP --\n";
echo "Expected EUC-JP encoded string in base64:\n";
var_dump(bin2hex($euc_jp_string));
echo "Converted Strings:\n";
var_dump(bin2hex(iconv('SJIS', 'EUC-JP', $sjis_string )));
var_dump(bin2hex(iconv('UTF-8', 'EUC-JP', $utf8_string)));

echo "\n-- Convert to SJIS --\n";
echo "Expected SJIS encoded string in base64:\n";
var_dump(bin2hex($sjis_string));
echo "Converted Strings:\n";
var_dump(bin2hex(iconv('EUC-JP', 'SJIS', $euc_jp_string)));
var_dump(bin2hex(iconv('UTF-8', 'SJIS', $utf8_string)));

echo "\n-- Convert to UTF-8 --\n";
echo "Expected UTF-8 encoded string in base64:\n";
var_dump(bin2hex($utf8_string));
echo "Converted Strings:\n";
var_dump(bin2hex(iconv('SJIS', 'UTF-8', $sjis_string)));
var_dump(bin2hex(iconv('EUC-JP', 'UTF-8', $euc_jp_string)));

echo "Done";
?>