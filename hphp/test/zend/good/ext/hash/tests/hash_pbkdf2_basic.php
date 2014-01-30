<?php

/* Prototype  : string hash_hmac  ( string $algo  , string $data  , string $key  [, bool $raw_output  ] )
 * Description: Generate a keyed hash value using the HMAC method
 * Source code: ext/hash/hash.c
 * Alias to functions:
*/

echo "*** Testing hash_pbkdf2() : basic functionality ***\n";

echo "sha1: " . hash_pbkdf2('sha1', 'password', 'salt', 1, 20)."\n";
echo "sha1(raw): " . bin2hex(hash_pbkdf2('sha1', 'password', 'salt', 1, 20, TRUE))."\n";
echo "sha1(rounds): " . hash_pbkdf2('sha1', 'passwordPASSWORDpassword', 'saltSALTsaltSALTsaltSALTsaltSALTsalt', 4096, 25)."\n";
echo "sha1(rounds)(raw): " . bin2hex(hash_pbkdf2('sha1', 'passwordPASSWORDpassword', 'saltSALTsaltSALTsaltSALTsaltSALTsalt', 4096, 25, TRUE))."\n";
echo "sha256: " . hash_pbkdf2('sha256', 'password', 'salt', 1, 20)."\n";
echo "sha256(raw): " . bin2hex(hash_pbkdf2('sha256', 'password', 'salt', 1, 20, TRUE))."\n";
echo "sha256(rounds): " . hash_pbkdf2('sha256', 'passwordPASSWORDpassword', 'saltSALTsaltSALTsaltSALTsaltSALTsalt', 4096, 40)."\n";
echo "sha256(rounds)(raw): " . bin2hex(hash_pbkdf2('sha256', 'passwordPASSWORDpassword', 'saltSALTsaltSALTsaltSALTsaltSALTsalt', 4096, 40, TRUE))."\n";

?>
===Done===