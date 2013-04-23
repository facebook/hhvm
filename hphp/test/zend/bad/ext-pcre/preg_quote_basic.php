<?php
/*
* proto string preg_quote(string str [, string delim_char])
* Function is implemented in ext/pcre/php_pcre.c
*/
$string_before = '/this *-has \ metacharacters^ in $';
print "\$string_before looks like: $string_before\n"; //$string_before is printed as is written
$string_after = preg_quote($string_before, '/');
print "\$string_after looks like: $string_after, with metacharacters and / (set as delimiter) escaped\n"; //$string_after is printed with metacharacters escaped.
$string1 = 'testing - /this *-has \ metacharacters^ in $ should   work';
var_dump(preg_match('/^[tT]\w{6} - ' . preg_quote($string_before, '/') . ' [a-z]*\s*work$/', $string1, $matches1));
var_dump($matches1);
?>