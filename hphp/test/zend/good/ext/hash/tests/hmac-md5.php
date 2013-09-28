<?php
/* Test Vectors from RFC 2104 */
$ctx = hash_init('md5',HASH_HMAC,str_repeat(chr(0x0b), 16));
hash_update($ctx, 'Hi There');
echo hash_final($ctx) . "\n";

$ctx = hash_init('md5',HASH_HMAC,'Jefe');
hash_update($ctx, 'what do ya want for nothing?');
echo hash_final($ctx) . "\n";

echo hash_hmac('md5', str_repeat(chr(0xDD), 50), str_repeat(chr(0xAA), 16)) . "\n";