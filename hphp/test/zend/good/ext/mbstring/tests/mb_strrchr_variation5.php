<?php
/* Prototype  : string mb_strrchr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds the last occurrence of a character in a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strrchr() : variation ***\n";

mb_internal_encoding('UTF-8');

//with repeated needles
$string_ascii = b'abcdef zbcdyx';
$needle_ascii = b"bcd";

//Japanese string in UTF-8 with repeated needles
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OIMzTvvJXvvJbml6XmnKzoqp7jg4bjgq3jgrnjg4g=');
$needle_mb = base64_decode('6Kqe44OG44Kt');

echo "-- Ascii data --\n";
var_dump(bin2hex(mb_strrchr($string_ascii, $needle_ascii, false)));
var_dump(bin2hex(mb_strrchr($string_ascii, $needle_ascii, true)));

echo "-- mb data in utf-8 --\n";
$res = mb_strrchr($string_mb, $needle_mb, false);
if ($res !== false) {
    var_dump(bin2hex($res));
}
else {
   echo "nothing found!\n";
}
$res = mb_strrchr($string_mb, $needle_mb, true);
if ($res !== false) {
    var_dump(bin2hex($res));
}
else {
   echo "nothing found!\n";
}


?>
===DONE===