<?php

/* Prototype  : string hash  ( string $algo  , string $data  [, bool $raw_output  ] )
 * Description: Generate a hash value (message digest)
 * Source code: ext/hash/hash.c
 * Alias to functions: 
*/
echo "*** Testing hash() : error conditions ***\n";

echo "\n-- Testing hash() function with less than expected no. of arguments --\n";
var_dump(hash());
var_dump(hash('adler32'));

echo "\n-- Testing hash() function with more than expected no. of arguments --\n";
$extra_arg= 10; 
var_dump(hash('adler32', '', false, $extra_arg));

echo "\n-- Testing hash() function with invalid hash algorithm --\n";
var_dump(hash('foo', ''));

?>
===Done===