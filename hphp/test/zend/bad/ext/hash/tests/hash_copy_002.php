<?php

$r = hash_init("md5");
var_dump(hash_copy());
var_dump(hash_copy($r));
var_dump(hash_copy($r, $r));

echo "Done\n";
?>