<?php

$c = hash_init("crc32");
hash_update($c, "Hello");
$d = hash_copy($c);
hash_update($c, "World");
hash_update($d, "Goodbye");
var_dump(hash_final($c));
var_dump(hash("crc32", "HelloWorld"));
var_dump(hash_final($d));
var_dump(hash("crc32", "HelloGoodbye"));

