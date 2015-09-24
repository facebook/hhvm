<?php

$ctx = hash_init('md5');
hash_update($ctx, 'The quick brown fox ');
hash_update($ctx, 'jumped over the lazy dog.');
echo hash_final($ctx);
$ctx2 = hash_copy($ctx);
var_dump($ctx2);
var_dump(hash_update($ctx, 'The quick brown fox '));
var_dump(hash_final($ctx));
