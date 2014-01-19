<?php

/* Prototype  : string hash_hmac  ( string $algo  , string $data  , string $key  [, bool $raw_output  ] )
 * Description: Generate a keyed hash value using the HMAC method
 * Source code: ext/hash/hash.c
 * Alias to functions: 
*/

echo "*** Testing hash_hmac() : basic functionality ***\n";

$content = "This is a sample string used to test the hash_hmac function with various hashing algorithms";
$key = 'secret';

echo "adler32: " . hash_hmac('adler32', $content, $key) . "\n";
echo "crc32: " . hash_hmac('crc32', $content, $key) . "\n";
echo "gost: " . hash_hmac('gost', $content, $key) . "\n";
echo "haval128,3: " . hash_hmac('haval128,3', $content, $key) . "\n";
echo "md2: " . hash_hmac('md2', $content, $key) . "\n";
echo "md4: " . hash_hmac('md4', $content, $key) . "\n";
echo "md5: " . hash_hmac('md5', $content, $key) . "\n";
echo "ripemd128: " . hash_hmac('ripemd128', $content, $key) . "\n";
echo "ripemd160: " . hash_hmac('ripemd160', $content, $key) . "\n";
echo "ripemd256: " . hash_hmac('ripemd256', $content, $key) . "\n";
echo "ripemd320: " . hash_hmac('ripemd320', $content, $key) . "\n";
echo "sha1: " . hash_hmac('sha1', $content, $key) . "\n";
echo "sha256: " . hash_hmac('sha256', $content, $key) . "\n";
echo "sha384: " . hash_hmac('sha384', $content, $key) . "\n";
echo "sha512: " . hash_hmac('sha512', $content, $key) . "\n";
echo "snefru: " . hash_hmac('snefru', $content, $key) . "\n";
echo "tiger192,3: " . hash_hmac('tiger192,3', $content, $key) . "\n";
echo "whirlpool: " . hash_hmac('whirlpool', $content, $key) . "\n";
echo "adler32(raw): " . bin2hex(hash_hmac('adler32', $content, $key, TRUE)) . "\n";
echo "md5(raw): " . bin2hex(hash_hmac('md5', $content, $key, TRUE)) . "\n";
echo "sha256(raw): " . bin2hex(hash_hmac('sha256', $content, $key, TRUE)) . "\n";

?>
===Done===