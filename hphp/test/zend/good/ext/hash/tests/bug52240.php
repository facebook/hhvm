<?php

$h = hash_init('crc32b', HASH_HMAC, '123456' );
$h2 = hash_copy($h);
var_dump(hash_final($h));
$h3 = hash_copy($h2);
var_dump(hash_final($h2));
var_dump(hash_final($h3));

?>