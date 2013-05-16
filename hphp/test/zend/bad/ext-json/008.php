<?php
$json = '{"largenum":123456789012345678901234567890}';
$x = json_decode($json);
var_dump($x->largenum);
$x = json_decode($json, false, 512, JSON_BIGINT_AS_STRING);
var_dump($x->largenum);
echo "Done\n";
?>