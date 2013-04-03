<?php

$s = "323423";
$s1 = "2323.555";

var_dump($s | $s1);
var_dump($s1 | $s);

$s = "some";
$s1 = "test";

var_dump($s | $s1);

$s = "some";
$s |= "test";

var_dump($s);

echo "Done\n";
?>