<?php

var_dump(realpath_cache_size());
$data = realpath_cache_get();
var_dump($data[__DIR__]);

echo "Done\n";
?>