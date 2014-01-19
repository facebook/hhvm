<?php
/* Prototype  : string imap_binary  ( string $string  )
 * Description: Convert an 8bit string to a base64 string.
 * Source code: ext/imap/php_imap.c
 */

echo "*** Testing imap_binary() : basic functionality ***\n";

echo "Encode as short string\n";
$str = b'This is an example string to be base 64 encoded';
$base64 = imap_binary($str);
var_dump(bin2hex($base64));

echo "Encode a string which results in more than 60 charters of output\n"; 
$str = b'This is a long string with results in more than 60 characters of output';
$base64 = imap_binary($str);
var_dump(bin2hex($base64));

echo "Encode a string with special characters\n";
$str = b'_+-={][];;@~#?/>.<,';
$base64 = imap_binary($str);
var_dump(bin2hex($base64));

echo "Encode some hexadecimal data\n";
$hex = b'x00\x01\x02\x03\x04\x05\x06\xFA\xFB\xFC\xFD\xFE\xFF';
$base64 = imap_binary($hex);
var_dump(bin2hex($base64));

?>
===Done===