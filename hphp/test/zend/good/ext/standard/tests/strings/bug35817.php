<?php
$a = pack("H3","181");
$b = unpack("H3", $a);
var_dump($b);

$a = pack("H2","18");
$b = unpack("H2", $a);
var_dump($b);

$a = pack("H","1");
$b = unpack("H", $a);
var_dump($b);
?>