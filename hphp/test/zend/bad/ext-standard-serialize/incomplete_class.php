<?php

$d = serialize(new __PHP_Incomplete_Class);
$o = unserialize($d);
var_dump($o);

$o->test = "a";
var_dump($o->test);
var_dump($o->test2);

echo "Done\n";
?>