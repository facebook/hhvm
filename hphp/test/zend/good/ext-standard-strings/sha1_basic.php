<?php

/* Prototype: string sha1  ( string $str  [, bool $raw_output  ] )
 * Description: Calculate the sha1 hash of a string
 */

echo "*** Testing sha1() : basic functionality ***\n";

echo "\n-- Without raw argument --\n";
var_dump(sha1(""));
var_dump(sha1("a"));
var_dump(sha1("abc"));
var_dump(sha1("message digest"));
var_dump(sha1("abcdefghijklmnopqrstuvwxyz"));
var_dump(sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
var_dump(sha1("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));

echo "\n-- With raw == false --\n";
var_dump(sha1("", false));
var_dump(sha1("a", false));
var_dump(sha1("abc", false));
var_dump(sha1("message digest", false));
var_dump(sha1("abcdefghijklmnopqrstuvwxyz", false));
var_dump(sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", false));
var_dump(sha1("12345678901234567890123456789012345678901234567890123456789012345678901234567890", false));

echo "\n-- With raw == true --\n";
var_dump(bin2hex(sha1("", true)));
var_dump(bin2hex(sha1("a", true)));
var_dump(bin2hex(sha1("abc", true)));
var_dump(bin2hex(sha1("message digest", true)));
var_dump(bin2hex(sha1("abcdefghijklmnopqrstuvwxyz", true)));
var_dump(bin2hex(sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", true)));
var_dump(bin2hex(sha1("12345678901234567890123456789012345678901234567890123456789012345678901234567890", true)));

?>
===DONE===