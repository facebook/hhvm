<?php
/* Prototype  : proto string base64_decode(string str[, bool strict])
 * Description: Decodes string using MIME base64 algorithm 
 * Source code: ext/standard/base64.c
 * Alias to functions: 
 */

echo "Decode an input string containing the whole base64 alphabet:\n";
$allbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
var_dump(bin2hex(base64_decode($allbase64)));
var_dump(bin2hex(base64_decode($allbase64, false)));
var_dump(bin2hex(base64_decode($allbase64, true)));

echo "Done";
?>