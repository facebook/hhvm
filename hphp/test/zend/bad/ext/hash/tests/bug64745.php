<?php
$hash = hash_pbkdf2('sha1', 'password', 'salt', 1, 0);
$rawHash = hash_pbkdf2('sha1', 'password', 'salt', 1, 0, true);

var_dump($hash);
var_dump(bin2hex($rawHash));

?>